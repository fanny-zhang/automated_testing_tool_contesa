; ModuleID = 'strcmp.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-f128:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

define i32 @strcmp(i8* %a, i8* %b) nounwind readonly {
entry:
  %a_addr = alloca i8*, align 8
  %b_addr = alloca i8*, align 8
  %retval = alloca i32
  %0 = alloca i32
  %"alloca point" = bitcast i32 0 to i32
  call void @llvm.dbg.declare(metadata !{i8** %a_addr}, metadata !9), !dbg !10
  store i8* %a, i8** %a_addr
  call void @llvm.dbg.declare(metadata !{i8** %b_addr}, metadata !11), !dbg !10
  store i8* %b, i8** %b_addr
  br label %bb1, !dbg !12

bb:                                               ; preds = %bb2
  %1 = load i8** %a_addr, align 8, !dbg !14
  %2 = getelementptr inbounds i8* %1, i64 1, !dbg !14
  store i8* %2, i8** %a_addr, align 8, !dbg !14
  %3 = load i8** %b_addr, align 8, !dbg !14
  %4 = getelementptr inbounds i8* %3, i64 1, !dbg !14
  store i8* %4, i8** %b_addr, align 8, !dbg !14
  br label %bb1, !dbg !14

bb1:                                              ; preds = %bb, %entry
  %5 = load i8** %a_addr, align 8, !dbg !12
  %6 = load i8* %5, align 1, !dbg !12
  %7 = icmp eq i8 %6, 0, !dbg !12
  br i1 %7, label %bb3, label %bb2, !dbg !12

bb2:                                              ; preds = %bb1
  %8 = load i8** %a_addr, align 8, !dbg !12
  %9 = load i8* %8, align 1, !dbg !12
  %10 = load i8** %b_addr, align 8, !dbg !12
  %11 = load i8* %10, align 1, !dbg !12
  %12 = icmp eq i8 %9, %11, !dbg !12
  br i1 %12, label %bb, label %bb3, !dbg !12

bb3:                                              ; preds = %bb2, %bb1
  %13 = load i8** %a_addr, align 8, !dbg !15
  %14 = load i8* %13, align 1, !dbg !15
  %15 = sext i8 %14 to i32, !dbg !15
  %16 = load i8** %b_addr, align 8, !dbg !15
  %17 = load i8* %16, align 1, !dbg !15
  %18 = sext i8 %17 to i32, !dbg !15
  %19 = sub nsw i32 %15, %18, !dbg !15
  store i32 %19, i32* %0, align 4, !dbg !15
  %20 = load i32* %0, align 4, !dbg !15
  store i32 %20, i32* %retval, align 4, !dbg !15
  br label %return, !dbg !15

return:                                           ; preds = %bb3
  %retval4 = load i32* %retval, !dbg !15
  ret i32 %retval4, !dbg !15
}

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

!llvm.dbg.sp = !{!0}

!0 = metadata !{i32 589870, i32 0, metadata !1, metadata !"strcmp", metadata !"strcmp", metadata !"strcmp", metadata !1, i32 10, metadata !3, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, i32 (i8*, i8*)* @strcmp} ; [ DW_TAG_subprogram ]
!1 = metadata !{i32 589865, metadata !"strcmp.c", metadata !"/home/tyu/Documents/bugredx/klee2.9/runtime/klee-libc/", metadata !2} ; [ DW_TAG_file_type ]
!2 = metadata !{i32 589841, i32 0, i32 1, metadata !"strcmp.c", metadata !"/home/tyu/Documents/bugredx/klee2.9/runtime/klee-libc/", metadata !"4.2.1 (Based on Apple Inc. build 5658) (LLVM build 2.9)", i1 true, i1 false, metadata !"", i32 0} ; [ DW_TAG_compile_unit ]
!3 = metadata !{i32 589845, metadata !1, metadata !"", metadata !1, i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !4, i32 0, null} ; [ DW_TAG_subroutine_type ]
!4 = metadata !{metadata !5, metadata !6, metadata !6}
!5 = metadata !{i32 589860, metadata !1, metadata !"int", metadata !1, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!6 = metadata !{i32 589839, metadata !1, metadata !"", metadata !1, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !7} ; [ DW_TAG_pointer_type ]
!7 = metadata !{i32 589862, metadata !1, metadata !"", metadata !1, i32 0, i64 8, i64 8, i64 0, i32 0, metadata !8} ; [ DW_TAG_const_type ]
!8 = metadata !{i32 589860, metadata !1, metadata !"char", metadata !1, i32 0, i64 8, i64 8, i64 0, i32 0, i32 6} ; [ DW_TAG_base_type ]
!9 = metadata !{i32 590081, metadata !0, metadata !"a", metadata !1, i32 10, metadata !6, i32 0} ; [ DW_TAG_arg_variable ]
!10 = metadata !{i32 10, i32 0, metadata !0, null}
!11 = metadata !{i32 590081, metadata !0, metadata !"b", metadata !1, i32 10, metadata !6, i32 0} ; [ DW_TAG_arg_variable ]
!12 = metadata !{i32 11, i32 0, metadata !13, null}
!13 = metadata !{i32 589835, metadata !0, i32 10, i32 0, metadata !1, i32 0} ; [ DW_TAG_lexical_block ]
!14 = metadata !{i32 12, i32 0, metadata !13, null}
!15 = metadata !{i32 13, i32 0, metadata !13, null}
