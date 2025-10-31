/** \file
 * Userspace interface to the BeagleBone PRU.
 *
 * Wraps the prussdrv library in a sane interface.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "pru.h"

#if ENABLE_PRU_UIO
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

static unsigned int proc_read(const char *const fname) {
  FILE *const f = fopen(fname, "r");
  if (!f)
    die("%s: Unable to open: %s", fname, strerror(errno));
  unsigned int x;
  fscanf(f, "%x", &x);
  fclose(f);
  return x;
}

pru_t *pru_init(const unsigned short pru_num) {
  prussdrv_init();

  int ret = prussdrv_open(PRU_EVTOUT_0);
  if (ret)
    die("prussdrv_open open failed\n");

  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
  prussdrv_pruintc_init(&pruss_intc_initdata);

  void *pru_data_mem;
  prussdrv_map_prumem(pru_num == 0 ? PRUSS0_PRU0_DATARAM : PRUSS0_PRU1_DATARAM,
                      &pru_data_mem);

  const int mem_fd = open("/dev/mem", O_RDWR);
  if (mem_fd < 0)
    die("Failed to open /dev/mem: %s\n", strerror(errno));

  const uintptr_t ddr_addr = proc_read("/sys/class/uio/uio0/maps/map1/addr");
  const uintptr_t ddr_size = proc_read("/sys/class/uio/uio0/maps/map1/size");

  const uintptr_t ddr_offset = ddr_addr;
  const size_t ddr_filelen = ddr_size;

  /* map the memory */
  uint8_t *const ddr_mem = (uint8_t*)mmap(0, ddr_filelen, PROT_WRITE | PROT_READ,
                                MAP_SHARED, mem_fd, ddr_offset);
  if (ddr_mem == MAP_FAILED)
    die("Failed to mmap offset %" PRIxPTR " @ %zu bytes: %s\n", ddr_offset,
        ddr_filelen, strerror(errno));

  close(mem_fd);

  pru_t *const pru = (pru_t*)calloc(1, sizeof(*pru));
  if (!pru)
    die("calloc failed: %s", strerror(errno));

  *pru = (pru_t) { .pru_num = pru_num,
                   .data_ram = pru_data_mem,
                   .data_ram_size = 8192, // how to determine?
                   .ddr = (void *)(ddr_mem),
                   .ddr_addr = ddr_addr,
                   .ddr_size = ddr_size, };

  printf("%s: PRU %d: data %p @ %zu bytes,  DMA %p / %" PRIxPTR
         " @ %zu bytes\n",
         __func__, pru_num, pru->data_ram, pru->data_ram_size, pru->ddr,
         pru->ddr_addr, pru->ddr_size);

  return pru;
}

void pru_exec(pru_t *const pru, const char *const program) {
  char *program_unconst = (char *)(uintptr_t)program;
  if (prussdrv_exec_program(pru->pru_num, program_unconst) < 0)
    die("%s failed", program);
}

void pru_exec_code(pru_t *const pru, const unsigned int* code, int codeLen) {
  if (prussdrv_exec_code(pru->pru_num, code, codeLen) < 0)
    die("prussdrv_exec_code() failed");
}

void pru_close(pru_t *const pru) {
  // \todo unmap memory
  prussdrv_pru_wait_event_timeout(PRU_EVTOUT_0, 100000);
  prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
  prussdrv_pru_disable(pru->pru_num);
  prussdrv_exit();
}

int pru_gpio(const unsigned gpio, const unsigned pin, const unsigned direction,
             const unsigned initial_value) {
  const unsigned pin_num = gpio * 32 + pin;
  const char *export_name = "/sys/class/gpio/export";
  char value_name[64];
  snprintf(value_name, sizeof(value_name), "/sys/class/gpio/gpio%u/value",
           pin_num);
  FILE* value = fopen(value_name, "w");
  if(!value)
  {
	FILE *const export_fd = fopen(export_name, "w");
	if (!export_fd)
		die("%s: Unable to open? %s\n", export_name, strerror(errno));

	fprintf(export_fd, "%d\n", pin_num);
	fclose(export_fd);
  }

  value = fopen(value_name, "w");
  if (!value)
    die("%s: Unable to open? %s\n", value_name, strerror(errno));

  fprintf(value, "%d\n", initial_value);
  fclose(value);

  char dir_name[64];
  snprintf(dir_name, sizeof(dir_name), "/sys/class/gpio/gpio%u/direction",
           pin_num);

  FILE *const dir = fopen(dir_name, "w");
  if (!dir)
    die("%s: Unable to open? %s\n", dir_name, strerror(errno));

  fprintf(dir, "%s\n", direction ? "out" : "in");
  fclose(dir);

  return 0;
}
#endif

#if ENABLE_PRU_RPROC
#include <PruManager.h>

#include <stdexcept>
#include <Bela.h>

struct PruPrivate {
	PruPrivate(unsigned short pru_num) : manager(pru_num, 0) {}
	PruManagerRprocMmap manager;
};
PruPrivate& p(pru_t* pru)
{
	if(!pru || !pru->p)
		throw std::runtime_error("PruPrivate: uninitialised\n");
	return *(PruPrivate*)pru->p;
}

pru_t *pru_init(const unsigned short pru_num) {
	pru_t* pru = new pru_t();
	pru->p = new PruPrivate(pru_num);
	auto& manager = p(pru).manager;
	pru->pru_num = pru_num;
	pru->data_ram = manager.getOwnMemory();
	pru->data_ram_size = 8192;
	unsigned int offset = 128; // let's use PRU's own ram for data
	pru->ddr = (uint8_t*)pru->data_ram + offset; //addr in ARM space
	pru->ddr_addr = offset; //addr in PRU space
	pru->ddr_size = pru->data_ram_size - offset;
	return pru;
}

void pru_exec_file(pru_t *const pru, char const* filename)
{
	if(p(pru).manager.start(filename))
		fprintf(stderr, "pru_exec_file: unable to start program %s\n", filename);
}

void pru_close(pru_t *const pru)
{
	p(pru).manager.stop();
	delete &p(pru);
	delete pru;
}

#include <Gpio.h>
static Gpio gpio;
int pru_gpio(unsigned gpio, unsigned pin, unsigned direction, const unsigned initial_value)
{
	int ret = ::gpio.open({(uint16_t)gpio, (uint16_t)pin}, 0 == direction ? Gpio::INPUT : Gpio::OUTPUT);
	if(0 == ret)
		::gpio.write(initial_value);
	return ret;
}

#endif