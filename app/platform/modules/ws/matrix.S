
    .section .com_text.gemm, "ax"
/*
    //void gemm_ikj_s(short* A, short* B, short* C, int m, int n, int k)
    .global gemm_ikj_s
gemm_ikj_s:

    mv      t0, a0
    mv      t3, a1
    li      a7, 0

0:  //loop 1
    mv      t4, a5
    mv      t2, t3
1:  //loop 2
    mv      t5, a4
    mv      t1, a2
    x.lhs   a0, 1(t0!)

2:  //loop 3
    x.lhs   a6, 0(t1!)
    x.lhs   a1, 1(t2!)
    x.macs  a7a6, a1, a0    //指针不好处理，暂不优化
    x.sh    a6, 1(t1!)

    addi    t5, t5, -1
    bgtz    t5, 2b

    addi    t4, t4, -1
    bgtz    t4, 1b

    sll     t6, a4, 1
    add     a2, a2, t6

    addi    a3, a3, -1
    bgtz    a3, 0b

    ret
*/

    //void gemm_ikj_s(s8* A, s8* B, s32* C, int m, int n, int k)
    .global gemm_ikj_s
gemm_ikj_s:

    mv      t0, a0
    mv      t3, a1
    li      a7, 0

0:  //loop 1
    mv      t4, a5
    mv      t2, t3
1:  //loop 2
    mv      t5, a4
    mv      t1, a2
    x.lbs   a0, 1(t0!)

2:  //loop 3
    x.lw   	a6, 0(t1!)
    x.lbs   a1, 1(t2!)
    x.macs  a7a6, a1, a0    //指针不好处理，暂不优化
    x.sw    a6, 1(t1!)

    addi    t5, t5, -1
    bgtz    t5, 2b

    addi    t4, t4, -1
    bgtz    t4, 1b

    sll     t6, a4, 2
    add     a2, a2, t6

    addi    a3, a3, -1
    bgtz    a3, 0b

    ret