//===-- X86FastISel.cpp - X86 FastISel implementation ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the X86-specific support for the FastISel class. Much
// of the target-specific code is generated by tablegen in the file
// X86GenFastISel.inc, which is #included here.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86ISelLowering.h"
#include "X86RegisterInfo.h"
#include "X86Subtarget.h"
#include "X86TargetMachine.h"
#include "llvm/InstrTypes.h"
#include "llvm/DerivedTypes.h"
#include "llvm/CodeGen/FastISel.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

class X86FastISel : public FastISel {
  /// Subtarget - Keep a pointer to the X86Subtarget around so that we can
  /// make the right decision when generating code for different targets.
  const X86Subtarget *Subtarget;
    
public:
  explicit X86FastISel(MachineFunction &mf,
                       DenseMap<const Value *, unsigned> &vm,
                       DenseMap<const BasicBlock *, MachineBasicBlock *> &bm)
    : FastISel(mf, vm, bm) {
    Subtarget = &TM.getSubtarget<X86Subtarget>();
  }

  virtual bool TargetSelectInstruction(Instruction *I);

#include "X86GenFastISel.inc"

private:
  bool X86SelectConstAddr(Value *V, unsigned &Op0);

  bool X86SelectLoad(Instruction *I);
  
  bool X86SelectStore(Instruction *I);

  bool X86SelectCmp(Instruction *I);
};

/// X86SelectConstAddr - Select and emit code to materialize constant address.
/// 
bool X86FastISel::X86SelectConstAddr(Value *V,
                                     unsigned &Op0) {
  // FIXME: Only GlobalAddress for now.
  GlobalValue *GV = dyn_cast<GlobalValue>(V);
  if (!GV)
    return false;

  if (Subtarget->GVRequiresExtraLoad(GV, TM, false)) {
    // Issue load from stub if necessary.
    unsigned Opc = 0;
    const TargetRegisterClass *RC = NULL;
    if (TLI.getPointerTy() == MVT::i32) {
      Opc = X86::MOV32rm;
      RC  = X86::GR32RegisterClass;
    } else {
      Opc = X86::MOV64rm;
      RC  = X86::GR64RegisterClass;
    }
    Op0 = createResultReg(RC);
    X86AddressMode AM;
    AM.GV = GV;
    addFullAddress(BuildMI(MBB, TII.get(Opc), Op0), AM);
    // Prevent loading GV stub multiple times in same MBB.
    LocalValueMap[V] = Op0;
  }
  return true;
}

/// X86SelectStore - Select and emit code to implement store instructions.
bool X86FastISel::X86SelectStore(Instruction* I) {
  MVT VT = MVT::getMVT(I->getOperand(0)->getType());
  if (VT == MVT::Other || !VT.isSimple())
    // Unhandled type.  Halt "fast" selection and bail.
    return false;
  if (VT == MVT::iPTR)
    // Use pointer type.
    VT = TLI.getPointerTy();
  // We only handle legal types. For example, on x86-32 the instruction
  // selector contains all of the 64-bit instructions from x86-64,
  // under the assumption that i64 won't be used if the target doesn't
  // support it.
  if (!TLI.isTypeLegal(VT))
    return false;
  unsigned Op0 = getRegForValue(I->getOperand(0));
  if (Op0 == 0)
    // Unhandled operand. Halt "fast" selection and bail.
    return false;    

  Value *V = I->getOperand(1);
  unsigned Op1 = getRegForValue(V);
  if (Op1 == 0) {
    // Handle constant load address.
    if (!isa<Constant>(V) || !X86SelectConstAddr(V, Op1))
      // Unhandled operand. Halt "fast" selection and bail.
      return false;    
  }
  
  // Get opcode and regclass of the output for the given load instruction.
  unsigned Opc = 0;
  const TargetRegisterClass *RC = NULL;
  switch (VT.getSimpleVT()) {
  default: return false;
  case MVT::i8:
    Opc = X86::MOV8mr;
    RC  = X86::GR8RegisterClass;
    break;
  case MVT::i16:
    Opc = X86::MOV16mr;
    RC  = X86::GR16RegisterClass;
    break;
  case MVT::i32:
    Opc = X86::MOV32mr;
    RC  = X86::GR32RegisterClass;
    break;
  case MVT::i64:
    // Must be in x86-64 mode.
    Opc = X86::MOV64mr;
    RC  = X86::GR64RegisterClass;
    break;
  case MVT::f32:
    if (Subtarget->hasSSE1()) {
      Opc = X86::MOVSSmr;
      RC  = X86::FR32RegisterClass;
    } else {
      Opc = X86::ST_Fp32m;
      RC  = X86::RFP32RegisterClass;
    }
    break;
  case MVT::f64:
    if (Subtarget->hasSSE2()) {
      Opc = X86::MOVSDmr;
      RC  = X86::FR64RegisterClass;
    } else {
      Opc = X86::ST_Fp64m;
      RC  = X86::RFP64RegisterClass;
    }
    break;
  case MVT::f80:
    Opc = X86::ST_FP80m;
    RC  = X86::RFP80RegisterClass;
    break;
  }

  X86AddressMode AM;
  if (Op1)
    // Address is in register.
    AM.Base.Reg = Op1;
  else
    AM.GV = cast<GlobalValue>(V);
  addFullAddress(BuildMI(MBB, TII.get(Opc)), AM).addReg(Op0);
  return true;
}

/// X86SelectLoad - Select and emit code to implement load instructions.
///
bool X86FastISel::X86SelectLoad(Instruction *I)  {
  MVT VT = MVT::getMVT(I->getType(), /*HandleUnknown=*/true);
  if (VT == MVT::Other || !VT.isSimple())
    // Unhandled type. Halt "fast" selection and bail.
    return false;
  if (VT == MVT::iPTR)
    // Use pointer type.
    VT = TLI.getPointerTy();
  // We only handle legal types. For example, on x86-32 the instruction
  // selector contains all of the 64-bit instructions from x86-64,
  // under the assumption that i64 won't be used if the target doesn't
  // support it.
  if (!TLI.isTypeLegal(VT))
    return false;

  Value *V = I->getOperand(0);
  unsigned Op0 = getRegForValue(V);
  if (Op0 == 0) {
    // Handle constant load address.
    if (!isa<Constant>(V) || !X86SelectConstAddr(V, Op0))
      // Unhandled operand. Halt "fast" selection and bail.
      return false;    
  }

  // Get opcode and regclass of the output for the given load instruction.
  unsigned Opc = 0;
  const TargetRegisterClass *RC = NULL;
  switch (VT.getSimpleVT()) {
  default: return false;
  case MVT::i8:
    Opc = X86::MOV8rm;
    RC  = X86::GR8RegisterClass;
    break;
  case MVT::i16:
    Opc = X86::MOV16rm;
    RC  = X86::GR16RegisterClass;
    break;
  case MVT::i32:
    Opc = X86::MOV32rm;
    RC  = X86::GR32RegisterClass;
    break;
  case MVT::i64:
    // Must be in x86-64 mode.
    Opc = X86::MOV64rm;
    RC  = X86::GR64RegisterClass;
    break;
  case MVT::f32:
    if (Subtarget->hasSSE1()) {
      Opc = X86::MOVSSrm;
      RC  = X86::FR32RegisterClass;
    } else {
      Opc = X86::LD_Fp32m;
      RC  = X86::RFP32RegisterClass;
    }
    break;
  case MVT::f64:
    if (Subtarget->hasSSE2()) {
      Opc = X86::MOVSDrm;
      RC  = X86::FR64RegisterClass;
    } else {
      Opc = X86::LD_Fp64m;
      RC  = X86::RFP64RegisterClass;
    }
    break;
  case MVT::f80:
    Opc = X86::LD_Fp80m;
    RC  = X86::RFP80RegisterClass;
    break;
  }

  unsigned ResultReg = createResultReg(RC);
  X86AddressMode AM;
  if (Op0)
    // Address is in register.
    AM.Base.Reg = Op0;
  else
    AM.GV = cast<GlobalValue>(V);
  addFullAddress(BuildMI(MBB, TII.get(Opc), ResultReg), AM);
  UpdateValueMap(I, ResultReg);
  return true;
}

bool X86FastISel::X86SelectCmp(Instruction *I) {
  CmpInst *CI = cast<CmpInst>(I);

  unsigned Op0Reg = getRegForValue(CI->getOperand(0));
  unsigned Op1Reg = getRegForValue(CI->getOperand(1));

  unsigned Opc;
  switch (TLI.getValueType(I->getOperand(0)->getType()).getSimpleVT()) {
  case MVT::i8: Opc = X86::CMP8rr; break;
  case MVT::i16: Opc = X86::CMP16rr; break;
  case MVT::i32: Opc = X86::CMP32rr; break;
  case MVT::i64: Opc = X86::CMP64rr; break;
  case MVT::f32: Opc = X86::UCOMISSrr; break;
  case MVT::f64: Opc = X86::UCOMISDrr; break;
  default: return false;
  }

  unsigned ResultReg = createResultReg(&X86::GR8RegClass);
  switch (CI->getPredicate()) {
  case CmpInst::FCMP_OEQ: {
    unsigned EReg = createResultReg(&X86::GR8RegClass);
    unsigned NPReg = createResultReg(&X86::GR8RegClass);
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETEr), EReg);
    BuildMI(MBB, TII.get(X86::SETNPr), NPReg);
    BuildMI(MBB, TII.get(X86::AND8rr), ResultReg).addReg(NPReg).addReg(EReg);
    break;
  }
  case CmpInst::FCMP_UNE: {
    unsigned NEReg = createResultReg(&X86::GR8RegClass);
    unsigned PReg = createResultReg(&X86::GR8RegClass);
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETNEr), NEReg);
    BuildMI(MBB, TII.get(X86::SETPr), PReg);
    BuildMI(MBB, TII.get(X86::OR8rr), ResultReg).addReg(PReg).addReg(NEReg);
    break;
  }
  case CmpInst::FCMP_OGT:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETAr), ResultReg);
    break;
  case CmpInst::FCMP_OGE:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETAEr), ResultReg);
    break;
  case CmpInst::FCMP_OLT:
    BuildMI(MBB, TII.get(Opc)).addReg(Op1Reg).addReg(Op0Reg);
    BuildMI(MBB, TII.get(X86::SETAr), ResultReg);
    break;
  case CmpInst::FCMP_OLE:
    BuildMI(MBB, TII.get(Opc)).addReg(Op1Reg).addReg(Op0Reg);
    BuildMI(MBB, TII.get(X86::SETAEr), ResultReg);
    break;
  case CmpInst::FCMP_ONE:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETNEr), ResultReg);
    break;
  case CmpInst::FCMP_ORD:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETNPr), ResultReg);
    break;
  case CmpInst::FCMP_UNO:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETPr), ResultReg);
    break;
  case CmpInst::FCMP_UEQ:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETEr), ResultReg);
    break;
  case CmpInst::FCMP_UGT:
    BuildMI(MBB, TII.get(Opc)).addReg(Op1Reg).addReg(Op0Reg);
    BuildMI(MBB, TII.get(X86::SETBr), ResultReg);
    break;
  case CmpInst::FCMP_UGE:
    BuildMI(MBB, TII.get(Opc)).addReg(Op1Reg).addReg(Op0Reg);
    BuildMI(MBB, TII.get(X86::SETBEr), ResultReg);
    break;
  case CmpInst::FCMP_ULT:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETBr), ResultReg);
    break;
  case CmpInst::FCMP_ULE:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETBEr), ResultReg);
    break;
  case CmpInst::ICMP_EQ:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETEr), ResultReg);
    break;
  case CmpInst::ICMP_NE:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETNEr), ResultReg);
    break;
  case CmpInst::ICMP_UGT:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETAr), ResultReg);
    break;
  case CmpInst::ICMP_UGE:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETAEr), ResultReg);
    break;
  case CmpInst::ICMP_ULT:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETBr), ResultReg);
    break;
  case CmpInst::ICMP_ULE:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETBEr), ResultReg);
    break;
  case CmpInst::ICMP_SGT:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETGr), ResultReg);
    break;
  case CmpInst::ICMP_SGE:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETGEr), ResultReg);
    break;
  case CmpInst::ICMP_SLT:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETLr), ResultReg);
    break;
  case CmpInst::ICMP_SLE:
    BuildMI(MBB, TII.get(Opc)).addReg(Op0Reg).addReg(Op1Reg);
    BuildMI(MBB, TII.get(X86::SETLEr), ResultReg);
    break;
  default:
    return false;
  }

  UpdateValueMap(I, ResultReg);
  return true;
}

bool
X86FastISel::TargetSelectInstruction(Instruction *I)  {
  switch (I->getOpcode()) {
  default: break;
  case Instruction::Load:
    return X86SelectLoad(I);
  case Instruction::Store:
    return X86SelectStore(I);
  case Instruction::ICmp:
  case Instruction::FCmp:
    return X86SelectCmp(I);
  }

  return false;
}

namespace llvm {
  llvm::FastISel *X86::createFastISel(MachineFunction &mf,
                        DenseMap<const Value *, unsigned> &vm,
                        DenseMap<const BasicBlock *, MachineBasicBlock *> &bm) {
    return new X86FastISel(mf, vm, bm);
  }
}
