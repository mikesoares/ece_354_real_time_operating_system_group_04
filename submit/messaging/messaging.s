	.globl asm_send_message
	.even
asm_send_message:
	jsr send_message_handler
	rte

	.globl asm_receive_message
	.even
asm_receive_message:
	jsr receive_message_handler
	rte

	.globl asm_delayed_send
	.even
asm_delayed_send:
	jsr delayed_send_handler
	rte
	
