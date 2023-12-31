## Bomblab Summary

这个实验需要一定的gdb知识，会几个常用指令就行，找个视频学一下。

最好把csapp的第三章读完，可以边读边做，前几个phase很简单。

### 1、phase1

```shell
   0x0000000000400ee0 <+0>:	sub    $0x8,%rsp
   0x0000000000400ee4 <+4>:	mov    $0x402400,%esi
   0x0000000000400ee9 <+9>:	callq  0x401338 <strings_not_equal>
   0x0000000000400eee <+14>:	test   %eax,%eax
   0x0000000000400ef0 <+16>:	je     0x400ef7 <phase_1+23>
   0x0000000000400ef2 <+18>:	callq  0x40143a <explode_bomb>
   0x0000000000400ef7 <+23>:	add    $0x8,%rsp
   0x0000000000400efb <+27>:	retq   
   -----------------------------------------------------------------------------
   先随便输入一个"dasdsadsadas"
   
   第9行callq 一般函数第一个参数为rdi，第二个为rsi（即esi）
   查询esi处和rdi处的值
   (gdb) x/s $esi
   0x402400:       "Border relations with Canada have never been better."
   (gdb) x/s $edi
   0x603780 <input_strings>:       "dasdsadsadas"
   
   把输入换成"Border relations with Canada have never been better."即可。
```

### 2、phase2

先把汇编代码取出来。

```shell
   0x0000000000400efc <+0>:	push   %rbp
   0x0000000000400efd <+1>:	push   %rbx
   0x0000000000400efe <+2>:	sub    $0x28,%rsp
   0x0000000000400f02 <+6>:	mov    %rsp,%rsi
   0x0000000000400f05 <+9>:	callq  0x40145c <read_six_numbers>    #很容易知道是读6个数字
   0x0000000000400f0a <+14>:	cmpl   $0x1,(%rsp)	#读取rsp及其后面几个int，可以发现就是我们的输入，这里要求第一个数为1
   0x0000000000400f0e <+18>:	je     0x400f30 <phase_2+52>
   0x0000000000400f10 <+20>:	callq  0x40143a <explode_bomb>
   0x0000000000400f15 <+25>:	jmp    0x400f30 <phase_2+52>
   0x0000000000400f17 <+27>:	mov    -0x4(%rbx),%eax
   0x0000000000400f1a <+30>:	add    %eax,%eax
   0x0000000000400f1c <+32>:	cmp    %eax,(%rbx)
   0x0000000000400f1e <+34>:	je     0x400f25 <phase_2+41>
   0x0000000000400f20 <+36>:	callq  0x40143a <explode_bomb>
   0x0000000000400f25 <+41>:	add    $0x4,%rbx
   0x0000000000400f29 <+45>:	cmp    %rbp,%rbx
   0x0000000000400f2c <+48>:	jne    0x400f17 <phase_2+27>
   0x0000000000400f2e <+50>:	jmp    0x400f3c <phase_2+64>
   0x0000000000400f30 <+52>:	lea    0x4(%rsp),%rbx
   0x0000000000400f35 <+57>:	lea    0x18(%rsp),%rbp
   ---------------------------------------------------------
   转换成c代码：
   for (int i = 1; i < 6; i++) {
   		if (nums[i + 1] != 2 * nums[i]) bomb();
   }
   所以输入的数字应该为1，2，4，8，16，32
   ---------------------------------------------------------
   0x0000000000400f3a <+62>:	jmp    0x400f17 <phase_2+27>
   0x0000000000400f3c <+64>:	add    $0x28,%rsp
   0x0000000000400f40 <+68>:	pop    %rbx
   0x0000000000400f41 <+69>:	pop    %rbp
   0x0000000000400f42 <+70>:	retq   

```

### 3、phase3

```shell
   0x0000000000400f43 <+0>:	sub    $0x18,%rsp
   0x0000000000400f47 <+4>:	lea    0xc(%rsp),%rcx
   0x0000000000400f4c <+9>:	lea    0x8(%rsp),%rdx
   0x0000000000400f51 <+14>:	mov    $0x4025cf,%esi		#查一下0x4025cf，发现要我们输入两个数字
   0x0000000000400f56 <+19>:	mov    $0x0,%eax
   0x0000000000400f5b <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000400f60 <+29>:	cmp    $0x1,%eax
   0x0000000000400f63 <+32>:	jg     0x400f6a <phase_3+39>
   0x0000000000400f65 <+34>:	callq  0x40143a <explode_bomb>
   0x0000000000400f6a <+39>:	cmpl   $0x7,0x8(%rsp)    #输出rsp + 0x8的值，发现为我们输入的第一个值，此处要求小于7
   0x0000000000400f6f <+44>:	ja     0x400fad <phase_3+106>
   0x0000000000400f71 <+46>:	mov    0x8(%rsp),%eax	    	
   0x0000000000400f75 <+50>:	jmpq   *0x402470(,%rax,8)   #此句有点类似于一个switch语句，根据eax的大小来跳转
   0x0000000000400f7c <+57>:	mov    $0xcf,%eax
   0x0000000000400f81 <+62>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f83 <+64>:	mov    $0x2c3,%eax
   0x0000000000400f88 <+69>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f8a <+71>:	mov    $0x100,%eax
   0x0000000000400f8f <+76>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f91 <+78>:	mov    $0x185,%eax
   0x0000000000400f96 <+83>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f98 <+85>:	mov    $0xce,%eax
   0x0000000000400f9d <+90>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f9f <+92>:	mov    $0x2aa,%eax
   0x0000000000400fa4 <+97>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fa6 <+99>:	mov    $0x147,%eax
   0x0000000000400fab <+104>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fad <+106>:	callq  0x40143a <explode_bomb>
   0x0000000000400fb2 <+111>:	mov    $0x0,%eax
   0x0000000000400fb7 <+116>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fb9 <+118>:	mov    $0x137,%eax
   0x0000000000400fbe <+123>:	cmp    0xc(%rsp),%eax        #输出rsp + 0xc的值发现是我们输入的第二个值，与我们从上面类似switch语句中取出到eax里的值来比较。
   0x0000000000400fc2 <+127>:	je     0x400fc9 <phase_3+134>
   0x0000000000400fc4 <+129>:	callq  0x40143a <explode_bomb>
   0x0000000000400fc9 <+134>:	add    $0x18,%rsp
   0x0000000000400fcd <+138>:	retq  
```

试了下第一个值填6，打断点后看看跳转到哪。

![](C:\Users\xuliz\Pictures\Screenshots\屏幕截图 2023-05-31 133942.png)

所以我们第二个值要为0x2aa（682）；

### 4、phase4

这题很好过，可以不用看func4这个函数，但是要理解汇编代码，还是要看一下。

先说下怎么过的把，可以看<+34><+39> 和 <+69><+74>，以下就能发现，第一个参数要小于14，第二个要为0，所以直接枚举就可以了，直接试了个 0，0就过了。

```shell
   0x000000000040100c <+0>:	sub    $0x18,%rsp
   0x0000000000401010 <+4>:	lea    0xc(%rsp),%rcx
   0x0000000000401015 <+9>:	lea    0x8(%rsp),%rdx
   0x000000000040101a <+14>:	mov    $0x4025cf,%esi
   0x000000000040101f <+19>:	mov    $0x0,%eax
   0x0000000000401024 <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000401029 <+29>:	cmp    $0x2,%eax
   0x000000000040102c <+32>:	jne    0x401035 <phase_4+41>
   0x000000000040102e <+34>:	cmpl   $0xe,0x8(%rsp)
   0x0000000000401033 <+39>:	jbe    0x40103a <phase_4+46>
   0x0000000000401035 <+41>:	callq  0x40143a <explode_bomb>
   0x000000000040103a <+46>:	mov    $0xe,%edx
   0x000000000040103f <+51>:	mov    $0x0,%esi
   0x0000000000401044 <+56>:	mov    0x8(%rsp),%edi
   0x0000000000401048 <+60>:	callq  0x400fce <func4>
   0x000000000040104d <+65>:	test   %eax,%eax
   0x000000000040104f <+67>:	jne    0x401058 <phase_4+76>
   0x0000000000401051 <+69>:	cmpl   $0x0,0xc(%rsp)
   0x0000000000401056 <+74>:	je     0x40105d <phase_4+81>
   0x0000000000401058 <+76>:	callq  0x40143a <explode_bomb>
   0x000000000040105d <+81>:	add    $0x18,%rsp
   0x0000000000401061 <+85>:	retq   
```

还是看一下func4具体发生了什么吧

```shell
   0x0000000000400fce <+0>:	sub    $0x8,%rsp
   0x0000000000400fd2 <+4>:	mov    %edx,%eax
   0x0000000000400fd4 <+6>:	sub    %esi,%eax
   0x0000000000400fd6 <+8>:	mov    %eax,%ecx
   0x0000000000400fd8 <+10>:	shr    $0x1f,%ecx
   0x0000000000400fdb <+13>:	add    %ecx,%eax
   0x0000000000400fdd <+15>:	sar    %eax
   0x0000000000400fdf <+17>:	lea    (%rax,%rsi,1),%ecx
   0x0000000000400fe2 <+20>:	cmp    %edi,%ecx
   0x0000000000400fe4 <+22>:	jle    0x400ff2 <func4+36>
   0x0000000000400fe6 <+24>:	lea    -0x1(%rcx),%edx
   0x0000000000400fe9 <+27>:	callq  0x400fce <func4>
   0x0000000000400fee <+32>:	add    %eax,%eax
   0x0000000000400ff0 <+34>:	jmp    0x401007 <func4+57>
   0x0000000000400ff2 <+36>:	mov    $0x0,%eax
   0x0000000000400ff7 <+41>:	cmp    %edi,%ecx
   0x0000000000400ff9 <+43>:	jge    0x401007 <func4+57>
   0x0000000000400ffb <+45>:	lea    0x1(%rcx),%esi
   0x0000000000400ffe <+48>:	callq  0x400fce <func4>
   0x0000000000401003 <+53>:	lea    0x1(%rax,%rax,1),%eax
   0x0000000000401007 <+57>:	add    $0x8,%rsp
   0x000000000040100b <+61>:	retq   
```



### 5、phase5

先把phase5的代码取出。

简单看一下可以发现<+24><+91>应该是输入字符串判断长度和判断是否相等

```shell
   0x0000000000401062 <+0>:	push   %rbx
   0x0000000000401063 <+1>:	sub    $0x20,%rsp
   0x0000000000401067 <+5>:	mov    %rdi,%rbx
   0x000000000040106a <+8>:	mov    %fs:0x28,%rax
   0x0000000000401073 <+17>:	mov    %rax,0x18(%rsp)
   0x0000000000401078 <+22>:	xor    %eax,%eax
   0x000000000040107a <+24>:	callq  0x40131b <string_length>
   0x000000000040107f <+29>:	cmp    $0x6,%eax   
   0x0000000000401082 <+32>:	je     0x4010d2 <phase_5+112>
   0x0000000000401084 <+34>:	callq  0x40143a <explode_bomb>
   --------------------------------------------------------------------
   此处应为判断长度是否为6，我们随便输入一个abcdef试试。
   --------------------------------------------------------------------
   0x0000000000401089 <+39>:	jmp    0x4010d2 <phase_5+112>
   0x000000000040108b <+41>:	movzbl (%rbx,%rax,1),%ecx
   0x000000000040108f <+45>:	mov    %cl,(%rsp)
   0x0000000000401092 <+48>:	mov    (%rsp),%rdx
   0x0000000000401096 <+52>:	and    $0xf,%edx
   0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx
   0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1)
   0x00000000004010a4 <+66>:	add    $0x1,%rax
   0x00000000004010a8 <+70>:	cmp    $0x6,%rax
   0x00000000004010ac <+74>:	jne    0x40108b <phase_5+41>
   0x00000000004010ae <+76>:	movb   $0x0,0x16(%rsp)
   0x00000000004010b3 <+81>:	mov    $0x40245e,%esi
   0x00000000004010b8 <+86>:	lea    0x10(%rsp),%rdi
   0x00000000004010bd <+91>:	callq  0x401338 <strings_not_equal>
   0x00000000004010c2 <+96>:	test   %eax,%eax
   0x00000000004010c4 <+98>:	je     0x4010d9 <phase_5+119>
   0x00000000004010c6 <+100>:	callq  0x40143a <explode_bomb>
   0x00000000004010cb <+105>:	nopl   0x0(%rax,%rax,1)
   0x00000000004010d0 <+110>:	jmp    0x4010d9 <phase_5+119>
   0x00000000004010d2 <+112>:	mov    $0x0,%eax
   0x00000000004010d7 <+117>:	jmp    0x40108b <phase_5+41>
   --------------------------------------------------------------------
   这其中应该是两重循环。
   查询0x4024b0的值
   (gdb) x/s 0x4024b0
   0x4024b0 <array.3449>:  "maduiersnfotvbylSo you think you can stop the bomb with c
   trl-c, do you?"
   一段奇怪的字符串。
   
   查询esi的值
   (gdb) x/s ($esi)
   0x40245e:       "flyers"
   这应该是需要让我们比较的字符串。
   
   查询rsp + 0x10的值
   (gdb) x/s ($rsp +0x10)
   0x7fffffffdeb0: "aduier"
   这似乎与那段奇怪的字符串有些联系，是其中的第2-7个字符，仔细看一下代码我们可以发现这段程序把我们输入的每个字符的低四位截取后    加上0x4024b0这个偏移量，取出其中的对应字符并存起来，所以我们需要根据字符的后4位来选取新字符串
   例如a：97-> 01100001,后四位为0001，所以选取的新字符为a
      b：98-> 01100010,后四位为0010，所以选取的新字符为d
   那么我们要想选取的字符串为"flyers",只需要另输入字符串为"IONEFG"(大写)，具体计算在此不列出。
   --------------------------------------------------------------------
   0x00000000004010d9 <+119>:	mov    0x18(%rsp),%rax
   0x00000000004010de <+124>:	xor    %fs:0x28,%rax
   0x00000000004010e7 <+133>:	je     0x4010ee <phase_5+140>
   0x00000000004010e9 <+135>:	callq  0x400b30 <__stack_chk_fail@plt>
   0x00000000004010ee <+140>:	add    $0x20,%rsp
   0x00000000004010f2 <+144>:	pop    %rbx
```



### 6、phase6

先把phase6的汇编代码取出来。

```shell
   0x00000000004010f4 <+0>:	push   %r14			
   0x00000000004010f6 <+2>:	push   %r13
   0x00000000004010f8 <+4>:	push   %r12
   0x00000000004010fa <+6>:	push   %rbp
   0x00000000004010fb <+7>:	push   %rbx
   0x00000000004010fc <+8>:	sub    $0x50,%rsp
   0x0000000000401100 <+12>:	mov    %rsp,%r13		#r13 = rsp
   0x0000000000401103 <+15>:	mov    %rsp,%rsi		#rsi = rsp
   0x0000000000401106 <+18>:	callq  0x40145c <read_six_numbers>
   0x000000000040110b <+23>:	mov    %rsp,%r14		#r14 = rsp
   0x000000000040110e <+26>:	mov    $0x0,%r12d		#r12d = 0
   ------------------------------------------------------------------------------------
   这里可以看出来应该是让我们读入6个数字
   ------------------------------------------------------------------------------------part 1
   0x0000000000401114 <+32>:	mov    %r13,%rbp		#rbp = r13 = rsp
   0x0000000000401117 <+35>:	mov    0x0(%r13),%eax	#eax = M[r13 + 0];
   0x000000000040111b <+39>:	sub    $0x1,%eax		#eax -= 1
   0x000000000040111e <+42>:	cmp    $0x5,%eax		
   0x0000000000401121 <+45>:	jbe    0x401128 <phase_6+52>	#if eax <= 5 to <+52>
   0x0000000000401123 <+47>:	callq  0x40143a <explode_bomb>   #else bomb!
   0x0000000000401128 <+52>:	add    $0x1,%r12d		#r12d += 1
   0x000000000040112c <+56>:	cmp    $0x6,%r12d		
   0x0000000000401130 <+60>:	je     0x401153 <phase_6+95>	#if r12d = 6 
   0x0000000000401132 <+62>:	mov    %r12d,%ebx		#ebx = r12d
   0x0000000000401135 <+65>:	movslq %ebx,%rax		#rax = ebx
   0x0000000000401138 <+68>:	mov    (%rsp,%rax,4),%eax	#eax = M[rsp + rax * 4]
   0x000000000040113b <+71>:	cmp    %eax,0x0(%rbp)		
   0x000000000040113e <+74>:	jne    0x401145 <phase_6+81>#if M[rbp] != eax go to <+81>
   0x0000000000401140 <+76>:	callq  0x40143a <explode_bomb> #else bomb!
   0x0000000000401145 <+81>:	add    $0x1,%ebx	#ebx++
   0x0000000000401148 <+84>:	cmp    $0x5,%ebx	
   0x000000000040114b <+87>:	jle    0x401135 <phase_6+65>	#if ebx <= 5 go to <+65>
   0x000000000040114d <+89>:	add    $0x4,%r13	#r13+=4
   0x0000000000401151 <+93>:	jmp    0x401114 <phase_6+32>	#go to <+32>
   ------------------------------------------------------------------------------------
   写成c语言：
   for (int i = 1; i <= 6; i++) {
   		if (nums[i] > 6) bomb();
       for (int j = 1; j <= 6; j++) {
           if (nums[i] == nums[j]) bomb();
       }
   }
   应该是让我们输入的数字在1-6的范围内且不重复
   ------------------------------------------------------------------------------------part 2
   0x0000000000401153 <+95>:	lea    0x18(%rsp),%rsi	#rsi = R[rsp] + 24
   0x0000000000401158 <+100>:	mov    %r14,%rax	#rax = r14:rsp
   0x000000000040115b <+103>:	mov    $0x7,%ecx	#ecx = 7
   0x0000000000401160 <+108>:	mov    %ecx,%edx	
   0x0000000000401162 <+110>:	sub    (%rax),%edx	#edx = 7 - M[rsp]
   0x0000000000401164 <+112>:	mov    %edx,(%rax)  #M[rsp]  = edx
   0x0000000000401166 <+114>:	add    $0x4,%rax	#rax += 4 : rsp上移4
   0x000000000040116a <+118>:	cmp    %rsi,%rax	
   0x000000000040116d <+121>:	jne    0x401160 <phase_6+108>  #if rsi != rax  goto <+108>
   ------------------------------------------------------------------------------------
  写成c语言
  for (int i = 1; i <= 6; i++) {
  		nums[i] = 7 - nums[i];
  }     
  应该是把我们输入的数字取了一个相反的顺序，比如原来是1，2，3，4，5，6，现在变成了6，5，4，3，2，1
  ------------------------------------------------------------------------------------part 3     
   0x000000000040116f <+123>:	mov    $0x0,%esi	#esi = 0
   0x0000000000401174 <+128>:	jmp    0x401197 <phase_6+163>
   0x0000000000401176 <+130>:	mov    0x8(%rdx),%rdx	# rdx = rdx + 8  ->(p = p.next,后移指针)
   0x000000000040117a <+134>:	add    $0x1,%eax	#eax = 2
   0x000000000040117d <+137>:	cmp    %ecx,%eax	
   0x000000000040117f <+139>:	jne    0x401176 <phase_6+130>
   0x0000000000401181 <+141>:	jmp    0x401188 <phase_6+148>
   0x0000000000401183 <+143>:	mov    $0x6032d0,%edx	#edx = 0x6032d0
   0x0000000000401188 <+148>:	mov    %rdx,0x20(%rsp,%rsi,2)	#M[rsp + 0 * 2 + 0x20] = rdx = 0x6032d0
   0x000000000040118d <+153>:	add    $0x4,%rsi	#rsi += 4
   0x0000000000401191 <+157>:	cmp    $0x18,%rsi	
   0x0000000000401195 <+161>:	je     0x4011ab <phase_6+183>	#if rsi == 0x18(24) to <+183>
   0x0000000000401197 <+163>:	mov    (%rsp,%rsi,1),%ecx	#ecx = M[rsp + rsi]
   0x000000000040119a <+166>:	cmp    $0x1,%ecx	
   0x000000000040119d <+169>:	jle    0x401183 <phase_6+143> #if ecx <= 1 to <+143>
   0x000000000040119f <+171>:	mov    $0x1,%eax	#eax = 1
   0x00000000004011a4 <+176>:	mov    $0x6032d0,%edx  #edx = 0x6032d0:0x6032d0为指向node1的指针位置
   0x00000000004011a9 <+181>:	jmp    0x401176 <phase_6+130>
   ------------------------------------------------------------------------------------   
   先把0x6032d0之后的数据显示出来
   
   	0x6032d0 <node1>:       0x0000014c      0x00000001      0x006032e0      0x00000000
						  332	
    0x6032e0 <node2>:       0x000000a8      0x00000002      0x006032f0      0x00000000
                            168
    0x6032f0 <node3>:       0x0000039c      0x00000003      0x00603300      0x00000000
                            924
    0x603300 <node4>:       0x000002b3      0x00000004      0x00603310      0x00000000
                            691
    0x603310 <node5>:       0x000001dd      0x00000005      0x00603320      0x00000000
                            477
    0x603320 <node6>:       0x000001bb      0x00000006      0x00000000      0x00000000
                            443
    此时我有点懵逼，这是个啥，不过根据查询网上的信息，得知这是个链表，结构如下
    
    struct node {
    	int val;（4字节）
    	int serialNumber;（4字节）
    	struct node *next;（8字节）
    }
    
    然后查询跑完此段后的rsp + 20后面的数据
    
    (gdb) x/12x ($rsp + 0x20)
    0x7fffffffde70: 0x006032f0      0x00000000      0x00603300      0x00000000
    0x7fffffffde80: 0x00603310      0x00000000      0x00603320      0x00000000
    0x7fffffffde90: 0x006032d0      0x00000000      0x006032e0      0x00000000
    
   可以发现程序把每个结点的位置放到了此处，应该是根据part2部分得出的反转后的顺序。
   --------------------------------------------------------------------------------part 4   
   0x00000000004011ab <+183>:	mov    0x20(%rsp),%rbx	#rbx = M[rsp + 0x20]  = 指向第一个node的指针
   0x00000000004011b0 <+188>:	lea    0x28(%rsp),%rax	#rax = R[rsp + 0x28] 
   0x00000000004011b5 <+193>:	lea    0x50(%rsp),%rsi	#rsi = R[rsp + 0x50]
   0x00000000004011ba <+198>:	mov    %rbx,%rcx	#rcx = rbx
   0x00000000004011bd <+201>:	mov    (%rax),%rdx	#rdx = M[rax]
   0x00000000004011c0 <+204>:	mov    %rdx,0x8(%rcx)	
   0x00000000004011c4 <+208>:	add    $0x8,%rax
   0x00000000004011c8 <+212>:	cmp    %rsi,%rax
   0x00000000004011cb <+215>:	je     0x4011d2 <phase_6+222>
   0x00000000004011cd <+217>:	mov    %rdx,%rcx
   0x00000000004011d0 <+220>:	jmp    0x4011bd <phase_6+201>
   ------------------------------------------------------------------------------------   
   此段在调的位置实际是0x6032d0处链表的内容，我们可以简单推断得出应该是在根据part3得到的顺序给链表重新排序，
   把断点打在后面，显示一下链表值。
    0x6032d0 <node1>:       0x0000014c      0x00000001      0x006032e0      0x00000000
    0x6032e0 <node2>:       0x000000a8      0x00000002      0x00000000      0x00000000
    0x6032f0 <node3>:       0x0000039c      0x00000003      0x00603300      0x00000000
    0x603300 <node4>:       0x000002b3      0x00000004      0x00603310      0x00000000
    0x603310 <node5>:       0x000001dd      0x00000005      0x00603320      0x00000000
    0x603320 <node6>:       0x000001bb      0x00000006      0x006032d0      0x00000000	
    可以发现指针的值确实被调整了。
   ------------------------------------------------------------------------------------part 5  
   0x00000000004011d2 <+222>:	movq   $0x0,0x8(%rdx)
   0x00000000004011da <+230>:	mov    $0x5,%ebp
   0x00000000004011df <+235>:	mov    0x8(%rbx),%rax
   0x00000000004011e3 <+239>:	mov    (%rax),%eax
   0x00000000004011e5 <+241>:	cmp    %eax,(%rbx)
   0x00000000004011e7 <+243>:	jge    0x4011ee <phase_6+250>
   0x00000000004011e9 <+245>:	callq  0x40143a <explode_bomb>
   0x00000000004011ee <+250>:	mov    0x8(%rbx),%rbx
   0x00000000004011f2 <+254>:	sub    $0x1,%ebp
   0x00000000004011f5 <+257>:	jne    0x4011df <phase_6+235>
   ------------------------------------------------------------------------------------   
   转换成c代码
   while (q -> next) {
   		node *p = q -> next;
   		if (p -> val > q -> val) bomb();
   		q = q -> next;
   }
   遍历链表是否为单调递减
   
   根据之前得到得结点值332，168，924，691，477，443
   					1，  2，  3，  4，  5，  6
   我们可以得出正确的顺序应为3，4，5，6，1，2
   那么我们的输入应该为4，3，2，1，6，5
   结束！
   ------------------------------------------------------------------------------------
   0x00000000004011f7 <+259>:	add    $0x50,%rsp
   0x00000000004011fb <+263>:	pop    %rbx
   0x00000000004011fc <+264>:	pop    %rbp
   0x00000000004011fd <+265>:	pop    %r12
   0x00000000004011ff <+267>:	pop    %r13
   0x0000000000401201 <+269>:	pop    %r14
   0x0000000000401203 <+271>:	retq   

```

给个图示吧。

![image-20230531114609561](C:\Users\xuliz\AppData\Roaming\Typora\typora-user-images\image-20230531114609561.png)