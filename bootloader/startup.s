
.global call_user_start

call_user_start:
	call0 main

/*
.global my_change_bbpll160
_br1:
	.long 0

my_change_bbpll160:
	addi a1, a1, -16
	
    addi	a1, a1, -16
	s32i.n	a0, a1, 0
//  l32r	a0, _br0
//	callx0	a0
//	l32r	a0, _br1  //pm_set_sleep_cycles
//	l8ui	a3, a0, 78
    l8ui	a0, a0, 178
	beqz.n	a3, 3695 <change_bbpll160+0x29>

    3682:	051066        	bnei	a0, 1, _ja0
    3685:	000005        	call0	_ja1
_ja1:
    3688:	000246        	j	_ja2
_ja0:
    368b:	60cc      	bnez.n	a0, 3695 <change_bbpll160+0x29>
    368d:	041226        	beqi	a2, 1, 3695 <change_bbpll160+0x29>
    3690:	128c      	beqz.n	a2, 3695 <change_bbpll160+0x29>
    3692:	000005        	call0	3694 <change_bbpll160+0x28>
_ja2:
    3695:	0108      	l32i.n	a0, a1, 0
    3697:	10c112        	addi	a1, a1, 16
    369a:	f00d      	ret.n
    369c:	005d      	mov.n	a5, a0
    369e:	000000        	ill
    36a1:	000000        	ill
    36a4:	000a00        	excw
    36a7:	020060        	excw
    36aa:	006000        	rsil	a0, 0
    36ad:	600006        	j	1b6b1 <phy_set_powerup_option+0x17429>
*/
