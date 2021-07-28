// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>
#include<inc/memlayout.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

  addr = ROUNDDOWN(addr, PGSIZE);
  if(!(err&FEC_WR) || !(uvpt[PGNUM(addr)]&PTE_COW)){
    panic("err is not FEC_WR or no PTE_COW err %08x pte %08x\n",err,uvpt[PGNUM(addr)]&PTE_COW);
  }

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
  sys_page_alloc(0, PFTEMP, PTE_P|PTE_U|PTE_W);
  memcpy(PFTEMP, addr, PGSIZE);
  sys_page_unmap(0, addr);
  sys_page_map(0, PFTEMP, 0, addr, PTE_P|PTE_U|PTE_W);
  sys_page_unmap(0, PFTEMP);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
  void* const addr = (void*)(pn<<PTXSHIFT);

	// LAB 4: Your code here.
  if(uvpt[pn]&PTE_SHARE) {
    r = sys_page_map(0, addr, envid, addr, (uvpt[pn]&PTE_SYSCALL));
    if(r<0) panic("failed in duppage, error: %e addr %08x pn %08x\n",r,addr,pn);
  } else if(uvpt[pn]&(PTE_W|PTE_COW)){
    r = sys_page_map(0, addr, envid, addr, ((uvpt[pn]&PTE_SYSCALL)&(~PTE_W))|PTE_COW);
    if(r<0) panic("failed in duppage, error: %e\n",r);
    r = sys_page_map(0, addr, 0, addr, ((uvpt[pn]&PTE_SYSCALL)&(~PTE_W))|PTE_COW);
    if(r<0) panic("failed in duppage, error: %e\n",r);
  } else {
    r = sys_page_map(0, addr, envid, addr, (uvpt[pn]&PTE_SYSCALL));
    if(r<0) panic("failed in duppage, error: %e addr %08x pn %08x\n",r,addr,pn);
  }
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
  set_pgfault_handler(pgfault);
  envid_t env = sys_exofork();
  if(env<0){
    return env;
  } else if(env==0){
    // child
		thisenv = &envs[ENVX(sys_getenvid())];
    return env;
  }
  // env>0 parent
  for(uintptr_t i=0;i<(uintptr_t)USTACKTOP;i+=PGSIZE) {
    if((uvpd[PDX(i)]&PTE_P) && (uvpt[PGNUM(i)]&PTE_P) && (uvpt[PGNUM(i)]&PTE_U)){
      duppage(env, PGNUM(i));
    }
  }
  int r = sys_page_alloc(env, (char*)(UXSTACKTOP-PGSIZE), PTE_P|PTE_U|PTE_W);
  if(r<0)panic("error alloc page %e",r);
  extern void _pgfault_upcall();
  r = sys_env_set_pgfault_upcall(env, _pgfault_upcall);
  if(r<0)panic("set pgfault upcall error %e\n",r);
  r = sys_env_set_status(env, ENV_RUNNABLE);
  if(r<0)panic("error set env status %e",r);
  return env;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
