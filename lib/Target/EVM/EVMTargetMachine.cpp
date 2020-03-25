//===-- EVMTargetMachine.cpp - Define TargetMachine for EVM -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about EVM target spec.
//
//===----------------------------------------------------------------------===//

#include "EVM.h"
#include "EVMTargetMachine.h"
#include "EVMTargetObjectFile.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
using namespace llvm;

extern "C" void LLVMInitializeEVMTarget() {
  RegisterTargetMachine<EVMTargetMachine> Y(getTheEVMTarget());
  auto PR = PassRegistry::getPassRegistry();

  initializeEVMConvertRegToStackPass(*PR);
  initializeEVMVRegToMemPass(*PR);
  initializeEVMAddJumpdestPass(*PR);
  initializeEVMShrinkpushPass(*PR);
  initializeEVMArgumentMovePass(*PR);
  initializeEVMExpandPseudosPass(*PR);
}

static std::string computeDataLayout(const Triple &TT) {
  // TODO: modify this.
  return "E-m:e-p:256:256-i:256:256-n256-S0";
}

static Reloc::Model getEffectiveRelocModel(const Triple &TT,
                                           Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::Static;
  return *RM;
}

EVMTargetMachine::EVMTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Optional<Reloc::Model> RM,
                                       Optional<CodeModel::Model> CM,
                                       CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(TT), TT, CPU, FS, Options,
                        getEffectiveRelocModel(TT, RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(make_unique<EVMELFTargetObjectFile>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}

namespace {
class EVMPassConfig : public TargetPassConfig {
public:
  EVMPassConfig(EVMTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  EVMTargetMachine &getEVMTargetMachine() const {
    return getTM<EVMTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
  void addPreRegAlloc() override;
  void addPostRegAlloc() override;

  FunctionPass *createTargetRegisterAllocator(bool) override;

};
}

TargetPassConfig *EVMTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new EVMPassConfig(*this, PM);
}

void EVMPassConfig::addIRPasses() {
  TargetPassConfig::addIRPasses();
  addPass(createEVMCallTransformation());
}

bool EVMPassConfig::addInstSelector() {
  TargetPassConfig::addInstSelector();
  addPass(createEVMISelDag(getEVMTargetMachine()));

  // we now uses a chain to make sure that the
  // STACKARGs are staying at the beginning of the code
  //addPass(createEVMArgumentMove());
  return false;
}

void EVMPassConfig::addPreEmitPass() {
  TargetPassConfig::addPreEmitPass();


  addPass(createEVMVRegToMem());
  addPass(createEVMExpandPseudos());

  if (getOptLevel() != CodeGenOpt::None) {
    //addPass(createEVMStackification());
  }

  addPass(createEVMConvertRegToStack());
  //addPass(createEVMAddJumpdest());
  addPass(createEVMShrinkpush());
  addPass(createEVMFinalization());
}

void EVMPassConfig::addPreRegAlloc() {
}

void EVMPassConfig::addPostRegAlloc() {
  // copied from WebAssembly backed.

  // These functions all require the NoVRegs property.
  disablePass(&MachineCopyPropagationID);
  disablePass(&PostRAMachineSinkingID);
  disablePass(&PostRASchedulerID);
  disablePass(&FuncletLayoutID);
  disablePass(&StackMapLivenessID);
  disablePass(&LiveDebugValuesID);
  disablePass(&PatchableFunctionID);
  disablePass(&ShrinkWrapID);

  TargetPassConfig::addPostRegAlloc();
}

// Disable register allocation.
FunctionPass *EVMPassConfig::createTargetRegisterAllocator(bool) {
  return nullptr; // No reg alloc
}

