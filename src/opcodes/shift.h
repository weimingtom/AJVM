

OPCODE(ishl) {
	int32_t a = JPOP(i32);
	int32_t b = JPOP(i32);

	JPUSH(i32, a << b);
}

OPCODE(lshl) {
	int64_t a = JPOP(i64);
	int32_t b = JPOP(i32);

	JPUSH(i64, a << b);
}


OPCODE(ishr) {
	int32_t a = JPOP(i32);
	int32_t b = JPOP(i32);

	JPUSH(i32, a >> b);
}

OPCODE(lshr) {
	int64_t a = JPOP(i64);
	int32_t b = JPOP(i32);

	JPUSH(i64, a >> b);
}


OPCODE(iushr) {
	uint32_t a = JPOP(u32);
	int32_t b = JPOP(i32);

	JPUSH(u32, a >> b);
}

OPCODE(lushr) {
	uint64_t a = JPOP(u64);
	int32_t b = JPOP(i32);

	JPUSH(u64, a >> b);
}
