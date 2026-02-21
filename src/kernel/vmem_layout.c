#include "vmem_layout.h"
#include "pmm.h"
#include "memory_helpers.h"
#include "multiboot.h"

extern uint8_t _kstart;
extern uint8_t _kend;

static vml_runtime_region_t bios_region;
static vml_runtime_region_t kernel_region;
static vml_runtime_region_t pmm_bitmap_region;
static vml_runtime_region_t vmem_temp_window_region;
static vml_runtime_region_t kernel_heap_region;

void vml_init(void)
{
	bios_region = (vml_runtime_region_t){
		.virt_base = 0xC0000000,
		.size = (0xC0100000 - 0xC0000000)
	};
	kernel_region = (vml_runtime_region_t){
		.virt_base = (size_t)&_kstart,
		.size = (size_t)ALIGN_UP((size_t)&_kend, PMM_PAGE_SIZE) - (size_t)&_kstart
	};
	pmm_bitmap_region = (vml_runtime_region_t){
		.virt_base = kernel_region.virt_base + kernel_region.size,
		.size = (size_t)ALIGN_UP(((size_t)get_highest_addr() / PMM_PAGE_SIZE + 7) / 8, PMM_PAGE_SIZE)
	};
	vmem_temp_window_region = (vml_runtime_region_t){
		.virt_base = pmm_bitmap_region.virt_base + pmm_bitmap_region.size,
		.size = PMM_PAGE_SIZE
	};
	kernel_heap_region = (vml_runtime_region_t){
		.virt_base = vmem_temp_window_region.virt_base + vmem_temp_window_region.size,
		.size = KERNEL_HEAP_SIZE
	};
}

vml_runtime_region_t vml_bios_region(void)
{
	return bios_region;
}

vml_runtime_region_t vml_kernel_region(void)
{
	return kernel_region;
}

vml_runtime_region_t vml_pmm_bitmap_region(void)
{
	return pmm_bitmap_region;
}

vml_runtime_region_t vml_vmem_temp_window_region(void)
{
	return vmem_temp_window_region;
}

vml_runtime_region_t vml_kernel_heap_region(void)
{
	return kernel_heap_region;
}
