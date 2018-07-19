# LIBRARY
PGDIR   = /usr/local/pgplot
CPGDIR  = $(PGDIR)
PGPLOT  = $(PGDIR)/libpgplot.a
CPGPLOT = $(CPGDIR)/libcpgplot.a
BINDIR  = /usr/local/custom/bin

# LINK OPTIONS
CFLAGS = -I/usr/local/pgplot -I/usr/X11R6/include
CCOMPL = gcc -g $(CFLAGS)
NVCC   = nvcc -I/usr/local/cuda/include -I/usr/local/cuda/common/inc
FCOMPL = gfortran 

# Followings are PASS or DIRECTORY
PROGS   = polaris_start shm_param shm_alloc shm_init cuda_fft_xspec shm_spec_view shm_xspec_view shm_phase_view shm_power_view k5sample_store PolariSplit PolariBunch
GRLIBS  = -L/usr/include/X11 -lX11
MATH    = -lm
FFTLIB  = -lcufft
CUDALIB = -lcutil

# MAPPING
OBJ_start          = polaris_start.o shm_access.o pow2round.o
OBJ_shm_param      = shm_param.o shm_init_create.o shm_access.o erase_shm.o
OBJ_shm_alloc      = shm_alloc.o shm_init_create.o shm_access.o
OBJ_shm_init       = shm_init.o shm_access.o
OBJ_spec_view      = shm_spec_view.o shm_access.o cpg_setup.o cpg_spec.o
OBJ_xspec_view     = shm_xspec_view.o shm_access.o cpg_setup.o cpg_xspec.o
OBJ_phase_view     = shm_phase_view.o shm_access.o cpg_setup.o cpg_phase.o
OBJ_power_view     = shm_power_view.o shm_access.o cpg_setup.o cpg_power.o
OBJ_k5sample_store = k5sample_store.o shm_access.o
OBJ_cuda_fft       = cuda_fft_xspec.o bitPower.o
OBJ_PolariSplit    = PolariSplit.o
OBJ_PolariBunch    = PolariBunch.o

# Compile and link
polaris_start: $(OBJ_start)
	$(CCOMPL) -o $@ $(OBJ_start)

k5sample_store: $(OBJ_k5sample_store)
	$(CCOMPL) -o $@ $(OBJ_k5sample_store)

shm_param: $(OBJ_shm_param)
	$(CCOMPL) -o $@ $(OBJ_shm_param)

shm_alloc: $(OBJ_shm_alloc)
	$(CCOMPL) -o $@ $(OBJ_shm_alloc)

shm_init: $(OBJ_shm_init)
	$(CCOMPL) -o $@ $(OBJ_shm_init)

shm_spec_view: $(OBJ_spec_view)
	$(FCOMPL) -o $@ $(OBJ_spec_view) $(CPGPLOT) $(PGPLOT) $(GRLIBS)

shm_xspec_view: $(OBJ_xspec_view)
	$(FCOMPL) -o $@ $(OBJ_xspec_view) $(CPGPLOT) $(PGPLOT) $(GRLIBS)

shm_phase_view: $(OBJ_phase_view)
	$(FCOMPL) -o $@ $(OBJ_phase_view) $(CPGPLOT) $(PGPLOT) $(GRLIBS)

shm_power_view: $(OBJ_power_view)
	$(FCOMPL) -o $@ $(OBJ_power_view) $(CPGPLOT) $(PGPLOT) $(GRLIBS)

cuda_fft_xspec: $(OBJ_cuda_fft)
	$(NVCC) -o $@ $(OBJ_cuda_fft) $(FFTLIB)

PolariSplit: $(OBJ_PolariSplit)
	$(CCOMPL) -o $@ $(OBJ_PolariSplit)

PolariBunch: $(OBJ_PolariBunch)
	$(CCOMPL) -o $@ $(OBJ_PolariBunch)

clean:
	\rm $(PROGS) *.o a.out core *.trace

all:
	$(PROGS)

install:
	@mv $(PROGS) $(BINDIR)

# Objects
cuda_fft_xspec.o: cuda_fft_xspec.cu	shm_k5data.inc cuda_polaris.inc
	$(NVCC) -c cuda_fft_xspec.cu
bitPower.o: bitPower.cu
	$(NVCC) -c bitPower.cu

.c.o:
	$(CCOMPL) -c $*.c

k5sample_store.o:  k5sample_store.c shm_k5data.inc
polaris_start.o:   polaris_start.c shm_k5data.inc
pow2round.o:       pow2round.c
PolariSplit.o:     PolariSplit.c shm_k5data.inc
PolariBunch.o:     PolariBunch.c shm_k5data.inc
shm_param.o:       shm_param.c shm_k5data.inc
shm_alloc.o:       shm_alloc.c shm_k5data.inc
shm_init.o:        shm_init.c shm_k5data.inc
erase_shm.o:       erase_shm.c shm_k5data.inc
shm_access.o:      shm_access.c shm_k5data.inc
shm_init_create.o: shm_init_create.c shm_k5data.inc
shm_spec_view.o:   shm_spec_view.c shm_k5data.inc
shm_xspec_view.o:  shm_xspec_view.c shm_k5data.inc
shm_phase_view.o:  shm_phase_view.c shm_k5data.inc
shm_power_view.o:  shm_power_view.c shm_k5data.inc
cpg_setup.o:       cpg_setup.c
cpg_spec.o:        cpg_spec.c
cpg_xspec.o:       cpg_xspec.c
cpg_phase.o:       cpg_phase.c
cpg_power.o:       cpg_power.c
