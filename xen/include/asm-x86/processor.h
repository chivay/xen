/*
 * include/asm-x86/processor.h
 *
 * Copyright (C) 1994 Linus Torvalds
 */

#ifndef __ASM_X86_PROCESSOR_H
#define __ASM_X86_PROCESSOR_H

#ifndef __ASSEMBLY__
#include <asm/page.h>
#include <asm/types.h>
#include <asm/cpufeature.h>
#include <asm/desc.h>
#include <asm/flushtlb.h>
#include <asm/pdb.h>
#include <xen/config.h>
#include <xen/spinlock.h>
#include <public/xen.h>
#endif

/*
 * CPU vendor IDs
 */
#define X86_VENDOR_INTEL 0
#define X86_VENDOR_CYRIX 1
#define X86_VENDOR_AMD 2
#define X86_VENDOR_UMC 3
#define X86_VENDOR_NEXGEN 4
#define X86_VENDOR_CENTAUR 5
#define X86_VENDOR_RISE 6
#define X86_VENDOR_TRANSMETA 7
#define X86_VENDOR_NSC 8
#define X86_VENDOR_SIS 9
#define X86_VENDOR_UNKNOWN 0xff

/*
 * EFLAGS bits
 */
#define X86_EFLAGS_CF	0x00000001 /* Carry Flag */
#define X86_EFLAGS_PF	0x00000004 /* Parity Flag */
#define X86_EFLAGS_AF	0x00000010 /* Auxillary carry Flag */
#define X86_EFLAGS_ZF	0x00000040 /* Zero Flag */
#define X86_EFLAGS_SF	0x00000080 /* Sign Flag */
#define X86_EFLAGS_TF	0x00000100 /* Trap Flag */
#define X86_EFLAGS_IF	0x00000200 /* Interrupt Flag */
#define X86_EFLAGS_DF	0x00000400 /* Direction Flag */
#define X86_EFLAGS_OF	0x00000800 /* Overflow Flag */
#define X86_EFLAGS_IOPL	0x00003000 /* IOPL mask */
#define X86_EFLAGS_NT	0x00004000 /* Nested Task */
#define X86_EFLAGS_RF	0x00010000 /* Resume Flag */
#define X86_EFLAGS_VM	0x00020000 /* Virtual Mode */
#define X86_EFLAGS_AC	0x00040000 /* Alignment Check */
#define X86_EFLAGS_VIF	0x00080000 /* Virtual Interrupt Flag */
#define X86_EFLAGS_VIP	0x00100000 /* Virtual Interrupt Pending */
#define X86_EFLAGS_ID	0x00200000 /* CPUID detection flag */

/*
 * Intel CPU flags in CR0
 */
#define X86_CR0_PE              0x00000001 /* Enable Protected Mode    (RW) */
#define X86_CR0_MP              0x00000002 /* Monitor Coprocessor      (RW) */
#define X86_CR0_EM              0x00000004 /* Require FPU Emulation    (RO) */
#define X86_CR0_TS              0x00000008 /* Task Switched            (RW) */
#define X86_CR0_NE              0x00000020 /* Numeric Error Reporting  (RW) */
#define X86_CR0_WP              0x00010000 /* Supervisor Write Protect (RW) */
#define X86_CR0_AM              0x00040000 /* Alignment Checking       (RW) */
#define X86_CR0_NW              0x20000000 /* Not Write-Through        (RW) */
#define X86_CR0_CD              0x40000000 /* Cache Disable            (RW) */
#define X86_CR0_PG              0x80000000 /* Paging                   (RW) */

/*
 * Intel CPU features in CR4
 */
#define X86_CR4_VME		0x0001	/* enable vm86 extensions */
#define X86_CR4_PVI		0x0002	/* virtual interrupts flag enable */
#define X86_CR4_TSD		0x0004	/* disable time stamp at ipl 3 */
#define X86_CR4_DE		0x0008	/* enable debugging extensions */
#define X86_CR4_PSE		0x0010	/* enable page size extensions */
#define X86_CR4_PAE		0x0020	/* enable physical address extensions */
#define X86_CR4_MCE		0x0040	/* Machine check enable */
#define X86_CR4_PGE		0x0080	/* enable global pages */
#define X86_CR4_PCE		0x0100	/* enable performance counters at ipl 3 */
#define X86_CR4_OSFXSR		0x0200	/* enable fast FPU save and restore */
#define X86_CR4_OSXMMEXCPT	0x0400	/* enable unmasked SSE exceptions */

/*
 * 'trap_bounce' flags values.
 */
#define TBF_TRAP        1
#define TBF_TRAP_NOCODE 2
#define TBF_TRAP_CR2    4

#ifndef __ASSEMBLY__

struct domain;

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */
#ifdef __x86_64__
#define current_text_addr() ({ void *pc; asm volatile("leaq 1f(%%rip),%0\n1:":"=r"(pc)); pc; })
#else
#define current_text_addr() \
  ({ void *pc; __asm__("movl $1f,%0\n1:":"=g" (pc)); pc; })
#endif

/*
 *  CPU type and hardware bug flags. Kept separately for each CPU.
 *  Members of this structure are referenced in head.S, so think twice
 *  before touching them. [mj]
 */

struct cpuinfo_x86 {
    __u8    x86;            /* CPU family */
    __u8    x86_vendor;     /* CPU vendor */
    __u8    x86_model;
    __u8    x86_mask;
    int     cpuid_level;    /* Maximum supported CPUID level, -1=no CPUID */
    __u32   x86_capability[NCAPINTS];
    char    x86_vendor_id[16];
    int     x86_cache_size;  /* in KB - for CPUS that support this call  */
    int	    x86_clflush_size;
    int	    x86_tlbsize;     /* number of 4K pages in DTLB/ITLB combined */
} __attribute__((__aligned__(SMP_CACHE_BYTES)));

/*
 * capabilities of CPUs
 */

extern struct cpuinfo_x86 boot_cpu_data;
extern struct tss_struct init_tss[NR_CPUS];

#ifdef CONFIG_SMP
extern struct cpuinfo_x86 cpu_data[];
#define current_cpu_data cpu_data[smp_processor_id()]
#else
#define cpu_data (&boot_cpu_data)
#define current_cpu_data boot_cpu_data
#endif

extern char ignore_irq13;

extern void identify_cpu(struct cpuinfo_x86 *);
extern void print_cpu_info(struct cpuinfo_x86 *);
extern void dodgy_tsc(void);

/*
 * Generic CPUID function
 */
static inline void cpuid(int op, int *eax, int *ebx, int *ecx, int *edx)
{
    __asm__("cpuid"
            : "=a" (*eax),
            "=b" (*ebx),
            "=c" (*ecx),
            "=d" (*edx)
            : "0" (op));
}

/*
 * CPUID functions returning a single datum
 */
static inline unsigned int cpuid_eax(unsigned int op)
{
    unsigned int eax;

    __asm__("cpuid"
            : "=a" (eax)
            : "0" (op)
            : "bx", "cx", "dx");
    return eax;
}
static inline unsigned int cpuid_ebx(unsigned int op)
{
    unsigned int eax, ebx;

    __asm__("cpuid"
            : "=a" (eax), "=b" (ebx)
            : "0" (op)
            : "cx", "dx" );
    return ebx;
}
static inline unsigned int cpuid_ecx(unsigned int op)
{
    unsigned int eax, ecx;

    __asm__("cpuid"
            : "=a" (eax), "=c" (ecx)
            : "0" (op)
            : "bx", "dx" );
    return ecx;
}
static inline unsigned int cpuid_edx(unsigned int op)
{
    unsigned int eax, edx;

    __asm__("cpuid"
            : "=a" (eax), "=d" (edx)
            : "0" (op)
            : "bx", "cx");
    return edx;
}


#define read_cr0() ({ \
	unsigned long __dummy; \
	__asm__( \
		"mov"__OS" %%cr0,%0\n\t" \
		:"=r" (__dummy)); \
	__dummy; \
})

#define write_cr0(x) \
	__asm__("mov"__OS" %0,%%cr0": :"r" (x));


/*
 * Save the cr4 feature set we're using (ie
 * Pentium 4MB enable and PPro Global page
 * enable), so that any CPU's that boot up
 * after us can get the correct flags.
 */
extern unsigned long mmu_cr4_features;

static inline void set_in_cr4 (unsigned long mask)
{
    mmu_cr4_features |= mask;
    __asm__("mov"__OS" %%cr4,%%"__OP"ax\n\t"
            "or"__OS" %0,%%"__OP"ax\n\t"
            "mov"__OS" %%"__OP"ax,%%cr4\n"
            : : "irg" (mask)
            :"ax");
}

static inline void clear_in_cr4 (unsigned long mask)
{
    mmu_cr4_features &= ~mask;
    __asm__("mov"__OS" %%cr4,%%"__OP"ax\n\t"
            "and"__OS" %0,%%"__OP"ax\n\t"
            "movl"__OS" %%"__OP"ax,%%cr4\n"
            : : "irg" (~mask)
            :"ax");
}

#define IOBMP_BYTES             8192
#define IOBMP_BYTES_PER_SELBIT  (IOBMP_BYTES / 64)
#define IOBMP_BITS_PER_SELBIT   (IOBMP_BYTES_PER_SELBIT * 8)
#define IOBMP_OFFSET            offsetof(struct tss_struct,io_bitmap)
#define IOBMP_INVALID_OFFSET    0x8000

struct i387_state {
    u8 state[512]; /* big enough for FXSAVE */
} __attribute__ ((aligned (16)));

struct tss_struct {
    unsigned short	back_link,__blh;
#ifdef __x86_64__
    u64 rsp0;
    u64 rsp1;
    u64 rsp2;
    u64 reserved1;
    u64 ist[7];
    u64 reserved2;
    u16 reserved3;
#else
    u32 esp0;
    u16 ss0,__ss0h;
    u32 esp1;
    u16 ss1,__ss1h;
    u32 esp2;
    u16 ss2,__ss2h;
    u32 __cr3;
    u32 eip;
    u32 eflags;
    u32 eax,ecx,edx,ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u16 es, __esh;
    u16 cs, __csh;
    u16 ss, __ssh;
    u16 ds, __dsh;
    u16 fs, __fsh;
    u16 gs, __gsh;
    u16 ldt, __ldth;
    u16 trace;
#endif
    u16 bitmap;
    u8  io_bitmap[IOBMP_BYTES];
    /* Pads the TSS to be cacheline-aligned (total size is 0x2080). */
    u32 __cacheline_filler[5];
};

struct trap_bounce {
    unsigned long  error_code;
    unsigned long  cr2;
    unsigned short flags; /* TBF_ */
    unsigned short cs;
    unsigned long  eip;
};

struct thread_struct {
    unsigned long      guestos_sp;
    unsigned long      guestos_ss;

    /* Hardware debugging registers */
    unsigned long      debugreg[8];  /* %%db0-7 debug registers */

    /* floating point info */
    struct i387_state  i387;

    /* general user-visible register state */
    execution_context_t user_ctxt;

    /*
     * Return vectors pushed to us by guest OS.
     * The stack frame for events is exactly that of an x86 hardware interrupt.
     * The stack frame for a failsafe callback is augmented with saved values
     * for segment registers %ds, %es, %fs and %gs:
     * 	%ds, %es, %fs, %gs, %eip, %cs, %eflags [, %oldesp, %oldss]
     */
    unsigned long event_selector;    /* 08: entry CS  */
    unsigned long event_address;     /* 12: entry EIP */

    unsigned long failsafe_selector; /* 16: entry CS  */
    unsigned long failsafe_address;  /* 20: entry EIP */

    /* Bounce information for propagating an exception to guest OS. */
    struct trap_bounce trap_bounce;

    /* I/O-port access bitmap. */
    u64 io_bitmap_sel; /* Selector to tell us which part of the IO bitmap are
                        * "interesting" (i.e. have clear bits) */
    u8 *io_bitmap; /* Pointer to task's IO bitmap or NULL */

    /* Trap info. */
#ifdef __i386__
    int                fast_trap_idx;
    struct desc_struct fast_trap_desc;
#endif
    trap_info_t        traps[256];
};

#define IDT_ENTRIES 256
extern struct desc_struct idt_table[];
extern struct desc_struct *idt_tables[];

#if defined(__i386__)

#define SET_DEFAULT_FAST_TRAP(_p) \
    (_p)->fast_trap_idx = 0x20;   \
    (_p)->fast_trap_desc.a = 0;   \
    (_p)->fast_trap_desc.b = 0;

#define CLEAR_FAST_TRAP(_p) \
    (memset(idt_tables[smp_processor_id()] + (_p)->fast_trap_idx, \
     0, 8))

#ifdef XEN_DEBUGGER
#define SET_FAST_TRAP(_p)   \
    (pdb_initialized ? (void *) 0 : \
       (memcpy(idt_tables[smp_processor_id()] + (_p)->fast_trap_idx, \
               &((_p)->fast_trap_desc), 8)))
#else
#define SET_FAST_TRAP(_p)   \
    (memcpy(idt_tables[smp_processor_id()] + (_p)->fast_trap_idx, \
            &((_p)->fast_trap_desc), 8))
#endif

long set_fast_trap(struct domain *p, int idx);

#define INIT_THREAD  { fast_trap_idx: 0x20 }

#elif defined(__x86_64__)

#define INIT_THREAD { 0 }

#endif /* __x86_64__ */

extern int gpf_emulate_4gb(struct xen_regs *regs);

struct mm_struct {
    /*
     * Every domain has a L1 pagetable of its own. Per-domain mappings
     * are put in this table (eg. the current GDT is mapped here).
     */
    l1_pgentry_t *perdomain_pt;
    pagetable_t  pagetable;

    /* shadow mode status and controls */
    unsigned int shadow_mode;  /* flags to control shadow table operation */
    pagetable_t  shadow_table;
    spinlock_t   shadow_lock;
    unsigned int shadow_max_page_count; // currently unused

    /* shadow hashtable */
    struct shadow_status *shadow_ht;
    struct shadow_status *shadow_ht_free;
    struct shadow_status *shadow_ht_extras; /* extra allocation units */
    unsigned int shadow_extras_count;

    /* shadow dirty bitmap */
    unsigned long *shadow_dirty_bitmap;
    unsigned int shadow_dirty_bitmap_size;  /* in pages, bit per page */

    /* shadow mode stats */
    unsigned int shadow_page_count;     
    unsigned int shadow_fault_count;     
    unsigned int shadow_dirty_count;     
    unsigned int shadow_dirty_net_count;     
    unsigned int shadow_dirty_block_count;     

    /* Current LDT details. */
    unsigned long ldt_base, ldt_ents, shadow_ldt_mapcnt;
    /* Next entry is passed to LGDT on domain switch. */
    char gdt[10]; /* NB. 10 bytes needed for x86_64. Use 6 bytes for x86_32. */
};

static inline void write_ptbase(struct mm_struct *mm)
{
    unsigned long pa;

    if ( unlikely(mm->shadow_mode) )
        pa = pagetable_val(mm->shadow_table);
    else
        pa = pagetable_val(mm->pagetable);

    write_cr3(pa);
}

#define IDLE0_MM                                                    \
{                                                                   \
    perdomain_pt: 0,                                                \
    pagetable:   mk_pagetable(__pa(idle_pg_table))                  \
}

/* Convenient accessor for mm.gdt. */
#define SET_GDT_ENTRIES(_p, _e) ((*(u16 *)((_p)->mm.gdt + 0)) = (((_e)<<3)-1))
#define SET_GDT_ADDRESS(_p, _a) ((*(unsigned long *)((_p)->mm.gdt + 2)) = (_a))
#define GET_GDT_ENTRIES(_p)     (((*(u16 *)((_p)->mm.gdt + 0))+1)>>3)
#define GET_GDT_ADDRESS(_p)     (*(unsigned long *)((_p)->mm.gdt + 2))

void destroy_gdt(struct domain *d);
long set_gdt(struct domain *d, 
             unsigned long *frames, 
             unsigned int entries);

long set_debugreg(struct domain *p, int reg, unsigned long value);

struct microcode {
    unsigned int hdrver;
    unsigned int rev;
    unsigned int date;
    unsigned int sig;
    unsigned int cksum;
    unsigned int ldrver;
    unsigned int pf;
    unsigned int reserved[5];
    unsigned int bits[500];
};

/* '6' because it used to be for P6 only (but now covers Pentium 4 as well) */
#define MICROCODE_IOCFREE	_IO('6',0)

/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
static inline void rep_nop(void)
{
    __asm__ __volatile__("rep;nop");
}

#define cpu_relax()	rep_nop()

/* Prefetch instructions for Pentium III and AMD Athlon */
#ifdef 	CONFIG_MPENTIUMIII

#define ARCH_HAS_PREFETCH
extern inline void prefetch(const void *x)
{
    __asm__ __volatile__ ("prefetchnta (%0)" : : "r"(x));
}

#elif CONFIG_X86_USE_3DNOW

#define ARCH_HAS_PREFETCH
#define ARCH_HAS_PREFETCHW
#define ARCH_HAS_SPINLOCK_PREFETCH

extern inline void prefetch(const void *x)
{
    __asm__ __volatile__ ("prefetch (%0)" : : "r"(x));
}

extern inline void prefetchw(const void *x)
{
    __asm__ __volatile__ ("prefetchw (%0)" : : "r"(x));
}
#define spin_lock_prefetch(x)	prefetchw(x)

#endif

void show_guest_stack();
void show_trace(unsigned long *esp);
void show_stack(unsigned long *esp);
void show_registers(struct xen_regs *regs);
asmlinkage void fatal_trap(int trapnr, struct xen_regs *regs, long error_code);

#endif /* !__ASSEMBLY__ */

#endif /* __ASM_X86_PROCESSOR_H */
