extern int  xprintf(uint8_t lineNo, const char *format, ...); 


void touchPanel_impl(void);
void setup_button(void);
void button_create(void);

typedef enum KEY_STATE { KEY_UNKNOWN, KEY_DOWN, KEY_UP};

typedef KEY_STATE *ptrKeyWrite;


void threeButtonMenu(
	char *leftButtonText, 
    ptrKeyWrite leftNotify,
	char *middleButtonText, 
	ptrKeyWrite middleNotify,
	char *rightButtonText, 
    ptrKeyWrite rightNotify
	);


void twoButtonMenu(
    char *leftButtonText, 
    ptrKeyWrite leftNotify,
    char *rightButtonText, 
    ptrKeyWrite rightNotify
    );



