	.globl asm_process_switch
	.even		
asm_process_switch:
	jsr	proc_switch_handler
	
	rte
	
	.globl asm_atomic_up
	.even		
asm_atomic_up:
	jsr atomic_up
	rte

	.globl asm_atomic_down
	.even
asm_atomic_down:
	jsr atomic_down
	rte
	
	.globl asm_set_priority
	.even
asm_set_priority:
	jsr set_process_priority_handler
	rte
	
	.globl asm_get_priority
	.even
asm_get_priority:
	jsr get_process_priority_handler
	rte

	
