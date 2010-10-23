#include "../shared/rtx_inc.h"
#include "../shared/rtx_io.h"
#include "../shared/rtx_mem.h"
#include "../shared/rtx_msg.h"
#include "../shared/rtx_pids.h"
#include "../shared/dbug.h"
#include "../shared/rtx_core.h"
#include "../shared/rtx_util.h"

void priority_switch_command_handler() 
{
	// register the command
	MessageEnvelope * kcd_msg = (MessageEnvelope *)request_memory_block();
	kcd_msg->type = TYPE_REGISTER_CMD;
	kcd_msg->msg[0] = '%';
	kcd_msg->msg[1] = 'C';
	send_message(KCD_PID, kcd_msg);

	// loop waiting for messages from the KCD
	while (1) {
		MessageEnvelope * cmd = receive_message(NULL);

		if (cmd->msg[0] != '%' || cmd->msg[1] != 'C' || cmd->msg[2] != ' ') {
			str_copy((BYTE *)"Command was invalid\n\r", (BYTE *)cmd->msg);
			send_message(CRT_PID, cmd);
			continue;
		}

		UINT32 i;

		// find the space in the argument and put a null there so ascii_to_int
		// can parse the two numbers independently
		for (i = 3; cmd->msg[i] != NULL; i ++) {
			if (cmd->msg[i] == ' ') {
				cmd->msg[i] = NULL;
				break;
			}
		}

		SINT32 pid = ascii_to_int(&cmd->msg[3]);
		SINT32 priority = ascii_to_int(&cmd->msg[i + 1]);

		if (pid == -1) {
			str_copy((BYTE *)"PID was invalid\n\r", (BYTE *)cmd->msg);
			send_message(CRT_PID, cmd);
			continue;
		}

		if (priority == -1) {
			str_copy((BYTE *)"Priority was invalid\n\r", (BYTE *)cmd->msg);
			send_message(CRT_PID, cmd);
			continue;
		}

		//rtx_dbug_outs_int("PID: ", pid);
		//rtx_dbug_outs_int("Priority: ", priority);

		int success = set_process_priority(pid, priority);

		if (success != 0) {
			str_copy((BYTE *)"Cannot change that process to that priority\n\r", (BYTE *)cmd->msg);
			send_message(CRT_PID, cmd);
			continue;
		}

		str_copy((BYTE *)"Priority change successful\n\r", (BYTE *)cmd->msg);
		send_message(CRT_PID, cmd);
	}
}

