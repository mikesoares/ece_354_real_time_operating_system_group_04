#include "rtx_inc.h"

VOID str_copy(BYTE * from, BYTE * to) {
	int i;
	for(i = 0; from[i] != '\0'; i++) {
		to[i] = from[i];
	}
}

SINT32 ascii_to_int(CHAR * string)
{
	SINT32 value = 0;

	// an empty string is an invalid string
	if (string[0] == NULL) {
		return -1;
	}

	int i;
	for (i = 0; string[i] != NULL; i++) {
		if (string[i] < 0x30 || string[i] > 0x39) {
			return -1;
		}
		
		value *= 10;
		value += (string[i] - 0x30);
	}

	return value;
}

