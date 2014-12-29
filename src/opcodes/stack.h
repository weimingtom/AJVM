

OPCODE(pop) {
	(void) JPOP_JV();
}

OPCODE(pop2) {
	(void) JPOP_JV();
	(void) JPOP_JV();
}

OPCODE(dup) {
	jvalue_t jv = JPOP_JV();
	
	JPUSH_JV(jv);
	JPUSH_JV(jv);
}

OPCODE(dup_x1) {
	jvalue_t jv1 = JPOP_JV();
	jvalue_t jv2 = JPOP_JV();

	JPUSH_JV(jv1);
	JPUSH_JV(jv2);
	JPUSH_JV(jv1);
}

OPCODE(dup_x2) {
	jvalue_t jv1 = JPOP_JV();
	jvalue_t jv2 = JPOP_JV();
	jvalue_t jv3 = JPOP_JV();
	

	JPUSH_JV(jv1);
	JPUSH_JV(jv2);
	JPUSH_JV(jv3);
	JPUSH_JV(jv1);
}

#define j_op_dup2 		j_op_dup
#define j_op_dup2_x1 	j_op_dup_x1
#define j_op_dup2_x2	j_op_dup_x2


OPCODE(swap) {
	jvalue_t jv1 = JPOP_JV();
	jvalue_t jv2 = JPOP_JV();

	JPUSH_JV(jv2);
	JPUSH_JV(jv1);
}