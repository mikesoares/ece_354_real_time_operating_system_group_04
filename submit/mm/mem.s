	.globl asm_request_memory_block
	.even
asm_request_memory_block:
	jsr request_memory_block_handler
	rte

	.globl asm_release_memory_block
	.even
asm_release_memory_block:
	jsr release_memory_handler
	rte
	