int main(void) { return 0; }

__attribute__((naked, noreturn)) void _reset(void) {
  extern long _sbss, _ebss, _sdata, _edata, _sidata;
  for (long *src = &_sbss; src < &_ebss; src++)
    *src = 0;
  for (long *src = &_sdata, *dst = &_sidata; src < &_edata; src++, dst++)
    *src = *dst;
  main();
  for (;;)
    (void)0;
}

extern void _estack(void);

/*
    Set tab (the vector table) in the section ".vectors"
    and the size of the vector table is 16 + 33
    the first 16 * 4 bytes are reserved for the Cortex-M0+ core
    the first one is the initial stack pointer
    the second one is the initial program counter
*/
__attribute__((section(".vectors"))) void (*tab[16 + 33])(void) = {_estack,
                                                                   _reset};
