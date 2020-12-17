#define STRLEN   16

struct A {
	struct B *ptr_a; // 
	char string_b[STRLEN]; // Any string
	char string_c[STRLEN]; // Any string
	char string_d[STRLEN]; // Must have vowel or add to end
	struct C *ptr_e; // 
	char string_f[STRLEN]; // Any string
	int (*op0)(struct A *objA);
	unsigned char *(*op1)(struct A *objA);
};
struct B {
	char string_a[STRLEN]; // Any string
	char string_b[STRLEN]; // Capitalize Strings
	char string_c[STRLEN]; // Any string
	int num_d; // >0 or set to 0
	int num_e; // >0 or set to 0
	char string_f[STRLEN]; // Any string
};
struct C {
	int num_a; // Any integer
	int num_b; // Any integer
	int num_c; // <0 or set to 0
	int num_d; // <0 or set to 0
};
