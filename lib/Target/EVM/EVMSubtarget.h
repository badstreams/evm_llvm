//===-- EVMSubtarget.h - Define Subtarget for the EVM -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the EVM specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_EVM_EVMSUBTARGET_H
#define LLVM_LIB_TARGET_EVM_EVMSUBTARGET_H

#include "MCTargetDesc/EVMMCTargetDesc.h"
#include "EVMFrameLowering.h"
#include "EVMISelLowering.h"
#include "EVMInstrInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

#define GET_SUBTARGETINFO_HEADER
#include "EVMGenSubtargetInfo.inc"

namespace llvm {
class StringRef;

class EVMSubtarget : public EVMGenSubtargetInfo {
  virtual void anchor();

  bool HasSubroutine = false;

  uint64_t AllocatedGlobalSlots;

  EVMFrameLowering FrameLowering;
  EVMInstrInfo InstrInfo;
  EVMTargetLowering TLInfo;
  SelectionDAGTargetInfo TSInfo;

  /// Initializes using the passed in CPU and feature strings so that we can
  /// use initializer lists for subtarget initialization.
  EVMSubtarget &initializeSubtargetDependencies(StringRef CPU, StringRef FS);

public:
  // Initializes the data members to match that of the specified triple.
  EVMSubtarget(const Triple &TT, const std::string &CPU,
               const std::string &FS, const TargetMachine &TM);

  bool hasSubroutine() const { return HasSubroutine; }

  void updateAllocatedGlobalSlots(uint64_t new_val) {
    AllocatedGlobalSlots = new_val;
  }

  // Parses features string setting specified subtarget options. The
  // definition of this function is auto-generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);

  const EVMFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }

  const EVMInstrInfo *getInstrInfo() const override { return &InstrInfo; }

  const EVMRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }

  const EVMTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }

  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }

  static unsigned get_push_opcode(unsigned length) {
    switch (length) {
    default:
      llvm_unreachable("incorrect size or unimplemented");
    case 1:
      return EVM::PUSH1;
    case 2:
      return EVM::PUSH2;
    case 3:
      return EVM::PUSH3;
    case 4:
      return EVM::PUSH4;
    case 5:
      return EVM::PUSH5;
    case 6:
      return EVM::PUSH6;
    case 7:
      return EVM::PUSH7;
    case 8:
      return EVM::PUSH8;
    case 9:
      return EVM::PUSH9;
    case 10:
      return EVM::PUSH10;
    case 11:
      return EVM::PUSH11;
    case 12:
      return EVM::PUSH12;
    case 13:
      return EVM::PUSH13;
    case 14:
      return EVM::PUSH14;
    case 15:
      return EVM::PUSH15;
    case 16:
      return EVM::PUSH16;
    case 17:
      return EVM::PUSH17;
    case 18:
      return EVM::PUSH18;
    case 19:
      return EVM::PUSH19;
    case 20:
      return EVM::PUSH20;
    case 21:
      return EVM::PUSH21;
    case 22:
      return EVM::PUSH22;
    case 23:
      return EVM::PUSH23;
    case 24:
      return EVM::PUSH24;
    case 25:
      return EVM::PUSH25;
    case 26:
      return EVM::PUSH26;
    case 27:
      return EVM::PUSH27;
    case 28:
      return EVM::PUSH28;
    case 29:
      return EVM::PUSH29;
    case 30:
      return EVM::PUSH30;
    case 31:
      return EVM::PUSH31;
    case 32:
      return EVM::PUSH32;
    }
  }

  static bool shouldSkipFunction(const Function *F) {
    return (F->isDeclaration() ||
            F->isIntrinsic() ||
            isMainFunction(*F)); // skip intrinsics.
  }

  static bool isMainFunction(const Function &F) {
    return F.getName() == StringRef("solidity.main") ||
           F.getName() == StringRef("main"); // skip intrinsics.
  }

  unsigned getFreeMemoryPointer() const { return (AllocatedGlobalSlots * 32); }
};
} // End llvm namespace

#endif
