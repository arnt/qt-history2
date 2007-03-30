/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
 * This file is created by using gcc sse intrinsics in C++ code, then
 * using g++ -S to create the assembly. The generated assembly code is then
 * modified to our needs.
 */

.globl _Z22qt_bitmapblit16_mmxextP13QRasterBufferiijPKhiii
	.type	_Z22qt_bitmapblit16_mmxextP13QRasterBufferiijPKhiii, @function
_Z22qt_bitmapblit16_mmxextP13QRasterBufferiijPKhiii:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	subl	$80, %esp
	movl	32(%ebp), %edx
	movzbl	22(%ebp), %ecx
	movl	$0, -40(%ebp)
	movl	24(%ebp), %eax
	movl	$0, -36(%ebp)
	movl	16(%ebp), %esi
	movl	%edx, -16(%ebp)
	movl	20(%ebp), %edx
	movl	$0, -48(%ebp)
	sall	$8, %ecx
	movl	%eax, -12(%ebp)
	andl	$-2048, %ecx
	movl	8(%ebp), %edi
	movl	$0, -44(%ebp)
	movl	%edx, %eax
	shrl	$5, %eax
	movl	$0, -56(%ebp)
	andl	$255, %edx
	andl	$2016, %eax
	movl	$0, -52(%ebp)
	shrl	$3, %edx
	orl	%eax, %edx
	movl	$0, -72(%ebp)
	orl	%ecx, %edx
	movl	8(%ebp), %ecx
	movl	$0, -68(%ebp)
	movzwl	%dx, %edx
	movl	48(%ecx), %ecx
	movl	%ecx, -24(%ebp)
	movl	%ecx, %eax
	imull	%esi, %eax
	movl	52(%edi), %ecx
	addl	%ecx, %eax
	movl	12(%ebp), %ecx
	leal	(%eax,%ecx,2), %eax
	movl	-24(%ebp), %ecx
	movl	%eax, -20(%ebp)
	movl	%edx, %eax
	sall	$16, %eax
	orl	%edx, %eax
	movl	$269492256, %edx
	shrl	%ecx
	movl	%eax, -32(%ebp)
	movl	%eax, -28(%ebp)
	movl	$1077969024, %eax
	movl	%eax, -80(%ebp)
	movl	$1077936128, %eax
	movl	%edx, -76(%ebp)
	movl	$1886412896, %edx
	movq	-80(%ebp), %mm0
	movq	-32(%ebp), %mm2
	movl	%eax, -80(%ebp)
	movl	%edx, -76(%ebp)
	movq	%mm0, %mm7
	cmpl	$4, 28(%ebp)
	movq	-80(%ebp), %mm0
	movq	%mm0, %mm1
	jle	.L291
	decl	-16(%ebp)
	movl	$67373064, -40(%ebp)
	movl	$16843266, -36(%ebp)
	cmpl	$-1, -16(%ebp)
	movl	$2088532088, -48(%ebp)
	movl	$2139061886, -44(%ebp)
	movq	-40(%ebp), %mm6
	movq	-48(%ebp), %mm0
	je	.L335
	addl	%ecx, %ecx
	movq	%mm1, %mm5
	movq	%mm2, %mm3
	movl	%ecx, -64(%ebp)
	movq	%mm0, %mm4
	.p2align 4,,15
.L314:
	xorl	%esi, %esi
	cmpl	28(%ebp), %esi
	jge	.L333
	movl	-20(%ebp), %ecx
	movl	%ecx, -84(%ebp)
	.p2align 4,,15
.L313:
	movl	-12(%ebp), %edx
	movl	%esi, %eax
	sarl	$3, %eax
	movzbl	(%eax,%edx), %eax
	testb	%al, %al
	je	.L300
	movzbl	%al, %edx
	movl	-84(%ebp), %edi
	movl	%edx, %eax
	sall	$8, %eax
	orl	%edx, %eax
	movl	%eax, %edx
	sall	$16, %edx
	orl	%eax, %edx
	leal	8(%ecx), %eax
	movl	%edx, -56(%ebp)
	movl	%edx, -52(%ebp)
	movq	-56(%ebp), %mm2
	movq	%mm2, %mm0
	pand	%mm7, %mm0
	paddw	%mm5, %mm0
	maskmovq	%mm0, %mm3
	movq	%mm2, %mm0
	pand	%mm6, %mm0
	movl	%eax, %edi
	paddw	%mm4, %mm0
	maskmovq	%mm0, %mm3
.L300:
	addl	$16, -84(%ebp)
	addl	$8, %esi
	addl	$16, %ecx
	cmpl	28(%ebp), %esi
	jl	.L313
.L333:
	decl	-16(%ebp)
	movl	-64(%ebp), %eax
	movl	36(%ebp), %edx
	addl	%eax, -20(%ebp)
	addl	%edx, -12(%ebp)
	cmpl	$-1, -16(%ebp)
	jne	.L314
.L335:
	femms
	addl	$80, %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.L291:
	decl	-16(%ebp)
	cmpl	$-1, -16(%ebp)
	je	.L335
	movq	%mm0, %mm3
	addl	%ecx, %ecx
	.p2align 4,,15
.L326:
	movl	-12(%ebp), %edi
	movzbl	(%edi), %eax
	testb	%al, %al
	je	.L318
	movzbl	%al, %edx
	movl	-20(%ebp), %edi
	movl	%edx, %eax
	sall	$8, %eax
	orl	%edx, %eax
	movl	%eax, %edx
	sall	$16, %edx
	orl	%eax, %edx
	movl	%edx, -72(%ebp)
	movl	%edx, -68(%ebp)
	movq	-72(%ebp), %mm0
	pand	%mm7, %mm0
	paddw	%mm3, %mm0
	maskmovq	%mm0, %mm2
.L318:
	decl	-16(%ebp)
	movl	36(%ebp), %eax
	addl	%ecx, -20(%ebp)
	addl	%eax, -12(%ebp)
	cmpl	$-1, -16(%ebp)
	jne	.L326
	femms
	addl	$80, %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	_Z22qt_bitmapblit16_mmxextP13QRasterBufferiijPKhiii, .-_Z22qt_bitmapblit16_mmxextP13QRasterBufferiijPKhiii

.globl _Z19qt_memfill32_mmxextPjji
	.type	_Z19qt_memfill32_mmxextPjji, @function
_Z19qt_memfill32_mmxextPjji:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	%edi, -4(%ebp)
	movl	16(%ebp), %edi
	movl	8(%ebp), %ecx
	movl	%ebx, -12(%ebp)
	call	__i686.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	%esi, -8(%ebp)
	cmpl	$6, %edi
	jg	.L244
	ja	.L273
	movl	.L252@GOTOFF(%ebx,%edi,4), %eax
	addl	%ebx, %eax
	jmp	*%eax
	.section	.rodata
	.align 4
	.align 4
.L252:
	.long	.L273@GOTOFF
	.long	.L251@GOTOFF
	.long	.L250@GOTOFF
	.long	.L249@GOTOFF
	.long	.L248@GOTOFF
	.long	.L247@GOTOFF
	.long	.L246@GOTOFF
	.text
	.p2align 4,,7
.L244:
	movl	12(%ebp), %eax
	movl	8(%ebp), %esi
	movl	%eax, -24(%ebp)
	movl	%eax, -20(%ebp)
	movl	%edi, %eax
	shrl	$31, %eax
	movq	-24(%ebp), %mm1
	leal	(%edi,%eax), %eax
	sarl	%eax
	movl	%eax, %edx
	leal	6(%eax), %ecx
	addl	$3, %edx
	cmovs	%ecx, %edx
	andl	$3, %eax
	sarl	$2, %edx
	cmpl	$1, %eax
	movq	%mm1, %mm0
	je	.L266
	jle	.L272
	cmpl	$2, %eax
	je	.L264
	cmpl	$3, %eax
	je	.L262
.L256:
	testl	$1, %edi
	je	.L269
	movl	12(%ebp), %edx
	movl	8(%ebp), %eax
	movl	%edx, -4(%eax,%edi,4)
.L269:
	femms
.L273:
	movl	-12(%ebp), %ebx
	movl	-8(%ebp), %esi
	movl	-4(%ebp), %edi
	movl	%ebp, %esp
	popl	%ebp
	ret
.L262:
	movl	%esi, %eax
	addl	$8, %esi
	movntq	%mm0, (%eax)
.L264:
	movl	%esi, %eax
	addl	$8, %esi
	movntq	%mm0, (%eax)
.L266:
	decl	%edx
	movl	%esi, %eax
	addl	$8, %esi
	movntq	%mm0, (%eax)
	testl	%edx, %edx
	jle	.L256
.L258:
	movl	%esi, %eax
	addl	$8, %esi
	movntq	%mm0, (%eax)
	jmp	.L262
.L246:
	movl	8(%ebp), %eax
	movl	12(%ebp), %edx
	movl	%eax, %ecx
	movl	%edx, (%eax)
	addl	$4, %ecx
.L247:
	movl	12(%ebp), %eax
	movl	%eax, (%ecx)
	addl	$4, %ecx
.L248:
	movl	12(%ebp), %edx
	movl	%edx, (%ecx)
	addl	$4, %ecx
.L249:
	movl	12(%ebp), %eax
	movl	%eax, (%ecx)
	addl	$4, %ecx
.L250:
	movl	12(%ebp), %edx
	movl	%edx, (%ecx)
	addl	$4, %ecx
.L251:
	movl	12(%ebp), %eax
	movl	%eax, (%ecx)
	movl	-12(%ebp), %ebx
	movl	-8(%ebp), %esi
	movl	-4(%ebp), %edi
	movl	%ebp, %esp
	popl	%ebp
	ret
.L272:
	testl	%eax, %eax
	je	.L258
	jmp	.L256
	.size	_Z19qt_memfill32_mmxextPjji, .-_Z19qt_memfill32_mmxextPjji

