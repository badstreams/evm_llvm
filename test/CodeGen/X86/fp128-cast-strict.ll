; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -disable-strictnode-mutation -mtriple=x86_64-linux-android -mattr=+sse | FileCheck %s --check-prefixes=X64,X64-SSE
; RUN: llc < %s -disable-strictnode-mutation -mtriple=x86_64-linux-gnu -mattr=+sse | FileCheck %s --check-prefixes=X64,X64-SSE
; RUN: llc < %s -disable-strictnode-mutation -mtriple=x86_64-linux-android -mattr=+avx | FileCheck %s --check-prefixes=X64,X64-AVX
; RUN: llc < %s -disable-strictnode-mutation -mtriple=x86_64-linux-gnu -mattr=+avx | FileCheck %s --check-prefixes=X64,X64-AVX
; RUN: llc < %s -disable-strictnode-mutation -mtriple=x86_64-linux-android -mattr=+avx512f | FileCheck %s --check-prefixes=X64,X64-AVX
; RUN: llc < %s -disable-strictnode-mutation -mtriple=x86_64-linux-gnu -mattr=+avx512f | FileCheck %s --check-prefixes=X64,X64-AVX

; Check soft floating point conversion function calls.

@vf32 = common global float 0.000000e+00, align 4
@vf64 = common global double 0.000000e+00, align 8
@vf80 = common global x86_fp80 0xK00000000000000000000, align 8
@vf128 = common global fp128 0xL00000000000000000000000000000000, align 16

define void @TestFPExtF32_F128() nounwind strictfp {
; X64-SSE-LABEL: TestFPExtF32_F128:
; X64-SSE:       # %bb.0: # %entry
; X64-SSE-NEXT:    pushq %rax
; X64-SSE-NEXT:    movss {{.*#+}} xmm0 = mem[0],zero,zero,zero
; X64-SSE-NEXT:    callq __extendsftf2
; X64-SSE-NEXT:    movaps %xmm0, {{.*}}(%rip)
; X64-SSE-NEXT:    popq %rax
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: TestFPExtF32_F128:
; X64-AVX:       # %bb.0: # %entry
; X64-AVX-NEXT:    pushq %rax
; X64-AVX-NEXT:    vmovss {{.*#+}} xmm0 = mem[0],zero,zero,zero
; X64-AVX-NEXT:    callq __extendsftf2
; X64-AVX-NEXT:    vmovaps %xmm0, {{.*}}(%rip)
; X64-AVX-NEXT:    popq %rax
; X64-AVX-NEXT:    retq
entry:
  %0 = load float, float* @vf32, align 4
  %conv = call fp128 @llvm.experimental.constrained.fpext.f128.f32(float %0, metadata !"fpexcept.strict") #0
  store fp128 %conv, fp128* @vf128, align 16
  ret void
}

define void @TestFPExtF64_F128() nounwind strictfp {
; X64-SSE-LABEL: TestFPExtF64_F128:
; X64-SSE:       # %bb.0: # %entry
; X64-SSE-NEXT:    pushq %rax
; X64-SSE-NEXT:    movsd {{.*#+}} xmm0 = mem[0],zero
; X64-SSE-NEXT:    callq __extenddftf2
; X64-SSE-NEXT:    movaps %xmm0, {{.*}}(%rip)
; X64-SSE-NEXT:    popq %rax
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: TestFPExtF64_F128:
; X64-AVX:       # %bb.0: # %entry
; X64-AVX-NEXT:    pushq %rax
; X64-AVX-NEXT:    vmovsd {{.*#+}} xmm0 = mem[0],zero
; X64-AVX-NEXT:    callq __extenddftf2
; X64-AVX-NEXT:    vmovaps %xmm0, {{.*}}(%rip)
; X64-AVX-NEXT:    popq %rax
; X64-AVX-NEXT:    retq
entry:
  %0 = load double, double* @vf64, align 8
  %conv = call fp128 @llvm.experimental.constrained.fpext.f128.f64(double %0, metadata !"fpexcept.strict") #0
  store fp128 %conv, fp128* @vf128, align 16
  ret void
}

define void @TestFPExtF80_F128() nounwind strictfp {
; X64-SSE-LABEL: TestFPExtF80_F128:
; X64-SSE:       # %bb.0: # %entry
; X64-SSE-NEXT:    subq $24, %rsp
; X64-SSE-NEXT:    fldt {{.*}}(%rip)
; X64-SSE-NEXT:    fstpt (%rsp)
; X64-SSE-NEXT:    callq __extendxftf2
; X64-SSE-NEXT:    movaps %xmm0, {{.*}}(%rip)
; X64-SSE-NEXT:    addq $24, %rsp
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: TestFPExtF80_F128:
; X64-AVX:       # %bb.0: # %entry
; X64-AVX-NEXT:    subq $24, %rsp
; X64-AVX-NEXT:    fldt {{.*}}(%rip)
; X64-AVX-NEXT:    fstpt (%rsp)
; X64-AVX-NEXT:    callq __extendxftf2
; X64-AVX-NEXT:    vmovaps %xmm0, {{.*}}(%rip)
; X64-AVX-NEXT:    addq $24, %rsp
; X64-AVX-NEXT:    retq
entry:
  %0 = load x86_fp80, x86_fp80* @vf80, align 8
  %conv = call fp128 @llvm.experimental.constrained.fpext.f128.f80(x86_fp80 %0, metadata !"fpexcept.strict") #0
  store fp128 %conv, fp128* @vf128, align 16
  ret void
}

define void @TestFPTruncF128_F32() nounwind strictfp {
; X64-SSE-LABEL: TestFPTruncF128_F32:
; X64-SSE:       # %bb.0: # %entry
; X64-SSE-NEXT:    pushq %rax
; X64-SSE-NEXT:    movaps {{.*}}(%rip), %xmm0
; X64-SSE-NEXT:    callq __trunctfsf2
; X64-SSE-NEXT:    movss %xmm0, {{.*}}(%rip)
; X64-SSE-NEXT:    popq %rax
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: TestFPTruncF128_F32:
; X64-AVX:       # %bb.0: # %entry
; X64-AVX-NEXT:    pushq %rax
; X64-AVX-NEXT:    vmovaps {{.*}}(%rip), %xmm0
; X64-AVX-NEXT:    callq __trunctfsf2
; X64-AVX-NEXT:    vmovss %xmm0, {{.*}}(%rip)
; X64-AVX-NEXT:    popq %rax
; X64-AVX-NEXT:    retq
entry:
  %0 = load fp128, fp128* @vf128, align 16
  %conv = call float @llvm.experimental.constrained.fptrunc.f32.f128(fp128 %0, metadata !"round.dynamic", metadata !"fpexcept.strict") #0
  store float %conv, float* @vf32, align 4
  ret void
}

define void @TestFPTruncF128_F64() nounwind strictfp {
; X64-SSE-LABEL: TestFPTruncF128_F64:
; X64-SSE:       # %bb.0: # %entry
; X64-SSE-NEXT:    pushq %rax
; X64-SSE-NEXT:    movaps {{.*}}(%rip), %xmm0
; X64-SSE-NEXT:    callq __trunctfdf2
; X64-SSE-NEXT:    movsd %xmm0, {{.*}}(%rip)
; X64-SSE-NEXT:    popq %rax
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: TestFPTruncF128_F64:
; X64-AVX:       # %bb.0: # %entry
; X64-AVX-NEXT:    pushq %rax
; X64-AVX-NEXT:    vmovaps {{.*}}(%rip), %xmm0
; X64-AVX-NEXT:    callq __trunctfdf2
; X64-AVX-NEXT:    vmovsd %xmm0, {{.*}}(%rip)
; X64-AVX-NEXT:    popq %rax
; X64-AVX-NEXT:    retq
entry:
  %0 = load fp128, fp128* @vf128, align 16
  %conv = call double @llvm.experimental.constrained.fptrunc.f64.f128(fp128 %0, metadata !"round.dynamic", metadata !"fpexcept.strict") #0
  store double %conv, double* @vf64, align 8
  ret void
}

define void @TestFPTruncF128_F80() nounwind strictfp {
; X64-SSE-LABEL: TestFPTruncF128_F80:
; X64-SSE:       # %bb.0: # %entry
; X64-SSE-NEXT:    pushq %rax
; X64-SSE-NEXT:    movaps {{.*}}(%rip), %xmm0
; X64-SSE-NEXT:    callq __trunctfxf2
; X64-SSE-NEXT:    fstpt {{.*}}(%rip)
; X64-SSE-NEXT:    popq %rax
; X64-SSE-NEXT:    retq
;
; X64-AVX-LABEL: TestFPTruncF128_F80:
; X64-AVX:       # %bb.0: # %entry
; X64-AVX-NEXT:    pushq %rax
; X64-AVX-NEXT:    vmovaps {{.*}}(%rip), %xmm0
; X64-AVX-NEXT:    callq __trunctfxf2
; X64-AVX-NEXT:    fstpt {{.*}}(%rip)
; X64-AVX-NEXT:    popq %rax
; X64-AVX-NEXT:    retq
entry:
  %0 = load fp128, fp128* @vf128, align 16
  %conv = call x86_fp80 @llvm.experimental.constrained.fptrunc.f80.f128(fp128 %0, metadata !"round.dynamic", metadata !"fpexcept.strict") #0
  store x86_fp80 %conv, x86_fp80* @vf80, align 8
  ret void
}

define i8 @fptosi_i8(fp128 %x) nounwind strictfp {
; X64-LABEL: fptosi_i8:
; X64:       # %bb.0: # %entry
; X64-NEXT:    pushq %rax
; X64-NEXT:    callq __fixtfsi
; X64-NEXT:    # kill: def $al killed $al killed $eax
; X64-NEXT:    popq %rcx
; X64-NEXT:    retq
entry:
  %conv = call i8 @llvm.experimental.constrained.fptosi.i8.f128(fp128 %x, metadata !"fpexcept.strict") #0
  ret i8 %conv
}

define i16 @fptosi_i16(fp128 %x) nounwind strictfp {
; X64-LABEL: fptosi_i16:
; X64:       # %bb.0: # %entry
; X64-NEXT:    pushq %rax
; X64-NEXT:    callq __fixtfsi
; X64-NEXT:    # kill: def $ax killed $ax killed $eax
; X64-NEXT:    popq %rcx
; X64-NEXT:    retq
entry:
  %conv = call i16 @llvm.experimental.constrained.fptosi.i16.f128(fp128 %x, metadata !"fpexcept.strict") #0
  ret i16 %conv
}

define i32 @fptosi_i32(fp128 %x) nounwind strictfp {
; X64-LABEL: fptosi_i32:
; X64:       # %bb.0: # %entry
; X64-NEXT:    pushq %rax
; X64-NEXT:    callq __fixtfsi
; X64-NEXT:    popq %rcx
; X64-NEXT:    retq
entry:
  %conv = call i32 @llvm.experimental.constrained.fptosi.i32.f128(fp128 %x, metadata !"fpexcept.strict") #0
  ret i32 %conv
}

define i64 @fptosi_i64(fp128 %x) nounwind strictfp {
; X64-LABEL: fptosi_i64:
; X64:       # %bb.0: # %entry
; X64-NEXT:    pushq %rax
; X64-NEXT:    callq __fixtfdi
; X64-NEXT:    popq %rcx
; X64-NEXT:    retq
entry:
  %conv = call i64 @llvm.experimental.constrained.fptosi.i64.f128(fp128 %x, metadata !"fpexcept.strict") #0
  ret i64 %conv
}

define i128 @fptosi_i128(fp128 %x) nounwind strictfp {
; X64-LABEL: fptosi_i128:
; X64:       # %bb.0: # %entry
; X64-NEXT:    pushq %rax
; X64-NEXT:    callq __fixtfti
; X64-NEXT:    popq %rcx
; X64-NEXT:    retq
entry:
  %conv = call i128 @llvm.experimental.constrained.fptosi.i128.f128(fp128 %x, metadata !"fpexcept.strict") #0
  ret i128 %conv
}

define i8 @fptoui_i8(fp128 %x) nounwind strictfp {
; X64-LABEL: fptoui_i8:
; X64:       # %bb.0: # %entry
; X64-NEXT:    pushq %rax
; X64-NEXT:    callq __fixtfsi
; X64-NEXT:    # kill: def $al killed $al killed $eax
; X64-NEXT:    popq %rcx
; X64-NEXT:    retq
entry:
  %conv = call i8 @llvm.experimental.constrained.fptoui.i8.f128(fp128 %x, metadata !"fpexcept.strict") #0
  ret i8 %conv
}

define i16 @fptoui_i16(fp128 %x) nounwind strictfp {
; X64-LABEL: fptoui_i16:
; X64:       # %bb.0: # %entry
; X64-NEXT:    pushq %rax
; X64-NEXT:    callq __fixtfsi
; X64-NEXT:    # kill: def $ax killed $ax killed $eax
; X64-NEXT:    popq %rcx
; X64-NEXT:    retq
entry:
  %conv = call i16 @llvm.experimental.constrained.fptoui.i16.f128(fp128 %x, metadata !"fpexcept.strict") #0
  ret i16 %conv
}

define i32 @fptoui_i32(fp128 %x) nounwind strictfp {
; X64-LABEL: fptoui_i32:
; X64:       # %bb.0: # %entry
; X64-NEXT:    pushq %rax
; X64-NEXT:    callq __fixunstfsi
; X64-NEXT:    popq %rcx
; X64-NEXT:    retq
entry:
  %conv = call i32 @llvm.experimental.constrained.fptoui.i32.f128(fp128 %x, metadata !"fpexcept.strict") #0
  ret i32 %conv
}

define i64 @fptoui_i64(fp128 %x) nounwind strictfp {
; X64-LABEL: fptoui_i64:
; X64:       # %bb.0: # %entry
; X64-NEXT:    pushq %rax
; X64-NEXT:    callq __fixunstfdi
; X64-NEXT:    popq %rcx
; X64-NEXT:    retq
entry:
  %conv = call i64 @llvm.experimental.constrained.fptoui.i64.f128(fp128 %x, metadata !"fpexcept.strict") #0
  ret i64 %conv
}

define i128 @fptoui_i128(fp128 %x) nounwind strictfp {
; X64-LABEL: fptoui_i128:
; X64:       # %bb.0: # %entry
; X64-NEXT:    pushq %rax
; X64-NEXT:    callq __fixunstfti
; X64-NEXT:    popq %rcx
; X64-NEXT:    retq
entry:
  %conv = call i128 @llvm.experimental.constrained.fptoui.i128.f128(fp128 %x, metadata !"fpexcept.strict") #0
  ret i128 %conv
}

attributes #0 = { strictfp }

declare float @llvm.experimental.constrained.fptrunc.f32.f128(fp128, metadata, metadata)
declare double @llvm.experimental.constrained.fptrunc.f64.f128(fp128, metadata, metadata)
declare x86_fp80 @llvm.experimental.constrained.fptrunc.f80.f128(fp128, metadata, metadata)
declare fp128 @llvm.experimental.constrained.fpext.f128.f32(float, metadata)
declare fp128 @llvm.experimental.constrained.fpext.f128.f64(double, metadata)
declare fp128 @llvm.experimental.constrained.fpext.f128.f80(x86_fp80, metadata)
declare i8 @llvm.experimental.constrained.fptosi.i8.f128(fp128, metadata)
declare i16 @llvm.experimental.constrained.fptosi.i16.f128(fp128, metadata)
declare i32 @llvm.experimental.constrained.fptosi.i32.f128(fp128, metadata)
declare i64 @llvm.experimental.constrained.fptosi.i64.f128(fp128, metadata)
declare i128 @llvm.experimental.constrained.fptosi.i128.f128(fp128, metadata)
declare i8 @llvm.experimental.constrained.fptoui.i8.f128(fp128, metadata)
declare i16 @llvm.experimental.constrained.fptoui.i16.f128(fp128, metadata)
declare i32 @llvm.experimental.constrained.fptoui.i32.f128(fp128, metadata)
declare i64 @llvm.experimental.constrained.fptoui.i64.f128(fp128, metadata)
declare i128 @llvm.experimental.constrained.fptoui.i128.f128(fp128, metadata)
