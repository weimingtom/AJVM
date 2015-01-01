#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>


#define _GNU_SOURCE
#include <float.h>
#include <math.h>

#include <assert.h>

#include <jvm/jvm.h>

#define _WITH_OPCODES
#include "opcodes/opcode.h"



int j_throw(jcontext_t* j, char* exception) {
	printf("Exception! %s\n", exception);

	if(!j || !j->code)
		exit(1);

	attr_linenumbers_t* ln = (attr_linenumbers_t*) jcode_find_attribute(j->current_assembly, j->method->code->attributes, "LineNumberTable");
	assert(ln);

	int i = 0;
	while(j->regs.pb >= ln->table[i].pc)
		i++;
	i--;

	printf("\tat %s:%d\n", j->method->name, ln->table[i].line);

#if defined(DEBUG)
	printf("\tat bytecode: [%d+%d] %s\n", j->regs.pb, j->regs.pc - j->regs.pb, j_opcodes[j->code[j->regs.pb]].name);

#if defined(VERBOSE)
	printf("\nStackdump:\n");
	printf("Size: %d; Position: %d\n", j->stack_size, j->stack_top);
	
	for(i = 0; i < j->stack_size; i++)
		printf(" [%d] %lld\n", i, j->stack[i].i64);
#endif
#endif

	exit(1);
}

int jcode_context_run(jcontext_t* j) {
	assert(j);

	if(setjmp(j->retbuf))
		return 0;

	j->regs.pc = 0;
	j->regs.pb = 0;

	while(1) {
		register int opcode = j->code[j->regs.pc];

#if defined(DEBUG)
		if(j_opcodes[opcode].handler == NULL) {
			j_printf("Wrong opcode: %X (%d) at %d\n", opcode, opcode, j->regs.pc);
			j_throw(j, JEXCEPTION_INVALID_OPCODE);
		}
#endif

#if defined(DEBUG) && defined(VERBOSE)
		j_printf("[%d] (%2X) %s\n", j->regs.pc, opcode, j_opcodes[opcode].name);
#endif
	
		j->regs.pb = j->regs.pc;
		j->regs.pc += 1;
		j_opcodes[opcode].handler (j);
	}

	return 0;
}

methodinfo_t* jcode_find_method(jassembly_t* j, const char* name) {
	if(j->header.jc_methods_count) {
		list_foreach(value, j->header.jc_methods) {
			methodinfo_t* v = (methodinfo_t*) value;

			cputf8_t utf;
			jclass_get_utf8_from_cp(j, &utf, v->name_index);
			
			if(strcmp(utf.value, name) == 0)
				return v;
		}
	}

	return NULL;
}

methodinfo_t* jcode_find_methodref(jassembly_t* j, int16_t idx) {
	assert(j);

	cpvalue_t* v = (cpvalue_t*) list_at_index(j->header.jc_cpinfo, idx - 1);
	assert(v);

	cpmethod_t mref;
	assert(jclass_cp_to_method(v, &mref) == 0);




	v = (cpvalue_t*) list_at_index(j->header.jc_cpinfo, mref.typename_index - 1);
	assert(v);

	cptypename_t ctype;
	assert(jclass_cp_to_typename(v, &ctype) == 0);


	cputf8_t utf;
	assert(jclass_get_utf8_from_cp(j, &utf, ctype.name_index) == 0);

	return jcode_find_method(j, utf.value);
}

attrinfo_t* jcode_find_attribute(jassembly_t* j, list_t* attributes, const char* name) {
	list_foreach(value, attributes) {
		attrinfo_t* v = (attrinfo_t*) value;


		cputf8_t utf1;
		jclass_get_utf8_from_cp(j, &utf1, v->name_index);
		
		if(strcmp(name, utf1.value) == 0)
			return v;
	}

	return NULL;
}


jvalue_t jcode_method_invoke(jassembly_t* j, methodinfo_t* method, jvalue_t* params, int params_count) {
	assert(j && method);

	if(params_count > 0)
		assert(params);


	if(method->access & ACC_NATIVE) {
		jnative_t* native = (jnative_t*) jnative_find_method(method->name);
		assert(native);
		
		if(!__builtin_expect((long int) native, 0))
			j_throw(NULL, "Native method not found");
		

		char* s = method->signature;
		register int i = 0, p = 0;
		while(s && params_count--) {
			switch(*s) {
				case 'B':
				case 'C':
				case 'F':
				case 'I':
				case 'L':
				case 'S':
				case 'Z':
				case 'V':
					__asm__ __volatile__ ("mov dword ptr [esp + edi], esi" : : "D"(i), "S"(params[p++].u32));
					i += 4; 
					break;
				case 'D':
				case 'J':
					__asm__ __volatile__ ("mov dword ptr [esp + edi], esi" : : "D"(i), "S"(params[p++].u32));
					__asm__ __volatile__ ("mov dword ptr [esp + edi + 4], esi" : : "D"(i), "S"((int32_t) (params[p++].u64 >> 32)));
					i += 8;
					break;
				case '[': {
						jarray_t* a = (jarray_t*) params[p++].ptr;
						register int j;

						
						switch(a->type) {
							case T_LONG:
							case T_DOUBLE:
								for(j = 0; j < a->length; j++) {
									__asm__ __volatile__ ("mov dword ptr [esp + edi], esi" : : "D"(i), "S"(a->data[j].u32));
									__asm__ __volatile__ ("mov dword ptr [esp + edi + 4], esi" : : "D"(i), "S"((int32_t) (a->data[j].u64 >> 32)));

									i += 8;
								}
							break;

							default:
								for(j = 0; j < a->length; j++) {
									__asm__ __volatile__ ("mov dword ptr [esp + edi], esi" : : "D"(i), "S"(a->data[j].u32));
									i += 4;
								}
						}	
					}
					break;		
				default:
					j_throw(NULL, "Invalid native method signature");
			}
	
			s++;
		}


		switch(method->rettype) {
			case T_LONG:
			case T_DOUBLE:
				return (jvalue_t) ((uint64_t (*) ()) native->handler) ();
			default:
				return (jvalue_t) ((uint32_t (*) ()) native->handler) ();
		}
	}



	if(!__builtin_expect((long int) method->code, 0))
		j_throw(NULL, "Invalid code for this method");


	jcontext_t* context = (jcontext_t*) jmalloc(sizeof(jcontext_t));
	context->current_assembly = j;

	context->locals = (jvalue_t*) jmalloc(sizeof(jvalue_t) * method->code->max_locals);
	context->locals_count = method->code->max_locals;

	context->stack = (jvalue_t*) jmalloc(sizeof(jvalue_t) * method->code->max_stack);
	context->stack_size = method->code->max_stack;
	context->stack_top = 0;

	context->code = method->code->code;
	context->method = method;

	int i, p;
	for(i = 0, p = 0; i < params_count; i++, p++) {

		context->locals[p] = params[i];

		switch(method->signature[i]) {
			case 'J':
			case 'D':
				p++;
				break;
		}	
	}		

#ifdef DEBUG
	j_printf("%s (args: %d; stack: %d; locals: %d)\n", method->name, method->nargs, method->code->max_stack, method->code->max_locals);
#endif


	jcode_context_run(context);
	jvalue_t retval = context->regs.r0;


#if !defined(__GLIBC__)
	jfree(context->locals);
	jfree(context->stack);
	jfree(context);
#endif

	return retval;
}


jvalue_t jcode_function_call(jassembly_t* j, const char* name, jvalue_t* params, int params_count) {
	assert(j && name);

	if(params_count > 0)
		assert(params);

	
	methodinfo_t* method = jcode_find_method(j, name);
	assert(method);

	return jcode_method_invoke(j, method, params, params_count);
}


