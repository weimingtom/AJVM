

static int __check_super(jassembly_t* child, jassembly_t* parent) {
	assert(child && parent);

	if(child->super == NULL)
		return -1;

	jassembly_t* tmp = child->super;
	while(tmp) {
		if(strcmp(tmp->name, parent->name) == 0)		
			return 0;		
		

		tmp = tmp->super;
	}

	return -1;
}

OPCODE(checkcast) {
	int16_t idx = PC16;
	PC += 2;

	jobject_t* obj = (jobject_t*) JPOP(ptr);
	if(!__builtin_expect((long int) obj, (long int) JCODE_NULL_REFERENCE)) {
		JPUSH(ptr, (void*) JCODE_NULL_REFERENCE);
		return;
	}

	cpvalue_t* v = (cpvalue_t*) list_at_index(j->current_assembly->header.jc_cpinfo, idx - 1);
	assert(v);

	cpclass_t cclass;
	assert(jclass_cp_to_class(v, &cclass) == 0);

	cputf8_t utf;
	assert(jclass_get_utf8_from_cp(j->current_assembly, &utf, cclass.name_index) == 0);

	jassembly_t* j2 = (jassembly_t*) jassembly_find(j->current_assembly, utf.value);
	assert(j2);

	
	if(!((strcmp(j2->name, obj->assembly->name) == 0) || (__check_super(obj->assembly, j2) == 0)))
		j_throw(j, JEXCEPTION_CAST_CLASS);


	JPUSH(ptr, (void*) obj);
}

OPCODE(instanceof) {
	int16_t idx = PC16;
	PC += 2;

	jobject_t* obj = (jobject_t*) JPOP(ptr);
	if(!__builtin_expect((long int) obj, (long int) JCODE_NULL_REFERENCE)) {
		JPUSH(i32, 0);
		return;
	}

	cpvalue_t* v = (cpvalue_t*) list_at_index(j->current_assembly->header.jc_cpinfo, idx - 1);
	assert(v);

	cpclass_t cclass;
	assert(jclass_cp_to_class(v, &cclass) == 0);

	cputf8_t utf;
	assert(jclass_get_utf8_from_cp(j->current_assembly, &utf, cclass.name_index) == 0);

	jassembly_t* j2 = (jassembly_t*) jassembly_find(j->current_assembly, utf.value);
	assert(j2);

	register int r = ((strcmp(j2->name, obj->assembly->name) == 0) || (__check_super(obj->assembly, j2) == 0));
	JPUSH(i32, r);
}
