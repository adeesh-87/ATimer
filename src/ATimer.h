typedef enum {
	AT_ONESHOT,
	AT_REPEATING
} ATimer_runtype;

struct ATimer_t;
typedef struct ATimer_t ATimer_t;

void ATimer_deinit(void);
void ATimer_init(void);
void ATimer_start(char *name, int period, ATimer_runtype type, void (*cb)(ATimer_t *, void *));
char *ATimer_getname(ATimer_t *ptr);
