#######################################################################

# Makefile for MAD-X development version

#######################################################################

# parameters

FCP=-O3 -fno-second-underscore -funroll-loops
FCM=-O2 -fno-second-underscore -funroll-loops
FCDB=-g -O0 -fno-second-underscore
FC=g77
f95=f95
GCC_FLAGS=-g -Wall -fno-second-underscore -D_CATCH_MEM
GCCP_FLAGS=-g -O3 -funroll-loops -fno-second-underscore -D_CATCH_MEM
f95_FLAGS=-gline -g90 -c -C=all -maxcontin=24 -nan
#f95_FLAGS=-c -O4 -maxcontin=24 -w=unused
FFLAGS77=-gline -g90 -c -maxcontin=24 -nan -ieee=full
#FFLAGS77=-g90 -c -O4 -maxcontin=24 -w=unused
CC=gcc
LIBX="-L/usr/X11R6/lib" -lX11
GLIB=/afs/cern.ch/group/si/slap/lib
GPUB=/afs/cern.ch/group/si/slap/bin


ifeq ($(OSTYPE),darwin)
# allows running of madx under Macinstosh System 10
# -fno-second-underscore  is old, do not use for more recent gnu compilers
# include headers for gxx11c
  GCCP_FLAGS=-g -O3 -funroll-loops -D_CATCH_MEM -I /usr/X11R6/include/
endif

default: madx

# files

twissp.o: twiss.F twiss0.fi twissa.fi twissl.fi twissc.fi twissotm.fi track.fi bb.fi name_len.fi twtrr.fi
	$(FC) $(FCP) -c -o twissp.o twiss.F

ptc_dummy.o: ptc_dummy.F
	$(FC) $(FCP) -c -o ptc_dummy.o ptc_dummy.F

surveyp.o: survey.F
	$(FC) $(FCP) -c -o surveyp.o survey.F

utilp.o: util.F twiss0.fi twtrr.fi
	$(FC) $(FCP) -c -o utilp.o util.F

dynapp.o: dynap.F deltra.fi dyntab.fi wmaxmin0.fi tunes.fi
	$(FC) $(FCP) -c -o dynapp.o dynap.F

madxnp.o: madxn.c madxu.c madxe.c madxc.c matchc.c sxf.c madx.h madxl.h madxd.h madxdict.h makethin.c c6t.c c6t.h
	$(CC) $(GCCP_FLAGS) -c -o madxnp.o madxn.c

ibsdbp.o: ibsdb.F ibsdb.fi name_len.fi physcons.fi
	$(FC) $(FCP) -c -o ibsdbp.o ibsdb.F

plotp.o: plot.F plot.fi plot_b.fi plot_c.fi plot_math.fi
	$(FC) $(FCP) -c -o plotp.o plot.F

trrunp.o: trrun.F twiss0.fi name_len.fi track.fi bb.fi twtrr.fi 
	$(FC) $(FCP) -c -o trrunp.o trrun.F

orbfp.o: orbf.F
	$(FC) $(FCP) -c -o orbfp.o orbf.F

emitp.o: emit.F twiss0.fi bb.fi emit.fi twtrr.fi
	$(FC) $(FCP) -c -o emitp.o emit.F

matchp.o: match.F name_len.fi match.fi 
	$(FC) $(FCP) -c -o matchp.o match.F

matchsap.o: matchsa.F
	$(FC) $(FCP) -c -o matchsap.o matchsa.F

gxx11p.o: gxx11.F
	$(FC) $(FCP) -c -o gxx11p.o gxx11.F

gxx11c.o: gxx11c.c
	$(CC) $(GCCP_FLAGS) -c -o gxx11c.o gxx11c.c

.F.o:
	$(f95) $(FFLAGS77) $<	

madxm.o: madxm.F
	$(f95) $(FFLAGS77) madxm.F

a_scratch_size.o: a_scratch_size.f90
	$(f95) $(f95_FLAGS) a_scratch_size.f90

b_da_arrays_all.o: a_scratch_size.o b_da_arrays_all.f90
	$(f95) $(f95_FLAGS) b_da_arrays_all.f90

c_dabnew.o: b_da_arrays_all.o c_dabnew.f90
	$(f95) $(f95_FLAGS) c_dabnew.f90

d_lielib.o: c_dabnew.o d_lielib.f90
	$(f95) $(f95_FLAGS) d_lielib.f90

e_define_newda.o: d_lielib.o e_define_newda.f90
	$(f95) $(f95_FLAGS) e_define_newda.f90

f_newda.o: e_define_newda.o f_newda.f90
	$(f95) $(f95_FLAGS) f_newda.f90

g_newLielib.o: f_newda.o g_newLielib.f90
	$(f95) $(f95_FLAGS) g_newLielib.f90

h_definition.o: g_newLielib.o h_definition.f90
	$(f95) $(f95_FLAGS) h_definition.f90

i_tpsa.o: h_definition.o i_tpsa.f90
	$(f95) $(f95_FLAGS) i_tpsa.f90

j_tpsalie.o: i_tpsa.o j_tpsalie.f90
	$(f95) $(f95_FLAGS) j_tpsalie.f90

k_tpsalie_analysis.o: j_tpsalie.o k_tpsalie_analysis.f90
	$(f95) $(f95_FLAGS) k_tpsalie_analysis.f90

l_complex_taylor.o: k_tpsalie_analysis.o l_complex_taylor.f90
	$(f95) $(f95_FLAGS) l_complex_taylor.f90

m_real_polymorph.o: l_complex_taylor.o m_real_polymorph.f90
	$(f95) $(f95_FLAGS) m_real_polymorph.f90

n_complex_polymorph.o: m_real_polymorph.o n_complex_polymorph.f90
	$(f95) $(f95_FLAGS) n_complex_polymorph.f90

Sa_extend_poly.o: n_complex_polymorph.o Sa_extend_poly.f90
	$(f95) $(f95_FLAGS) Sa_extend_poly.f90

Sb_1_pol_template.o: Sa_extend_poly.o Sb_1_pol_template.f90
	$(f95) $(f95_FLAGS) Sb_1_pol_template.f90

Sb_2_pol_template.o: Sb_1_pol_template.o Sb_2_pol_template.f90
	$(f95) $(f95_FLAGS) Sb_2_pol_template.f90

Sc_euclidean.o: Sb_2_pol_template.o Sc_euclidean.f90
	$(f95) $(f95_FLAGS) Sc_euclidean.f90

Sd_frame.o: Sc_euclidean.o Sd_frame.f90
	$(f95) $(f95_FLAGS) Sd_frame.f90

Se_status.o: Sd_frame.o Se_status.f90
	$(f95) $(f95_FLAGS) Se_status.f90

Sf_def_all_kinds.o: Se_status.o Sf_def_all_kinds.f90 \
	a_def_all_kind.inc a_def_user1.inc a_def_user2.inc \
	a_def_element_fibre_layout.inc
	$(f95) $(f95_FLAGS) Sf_def_all_kinds.f90

Sg_0_fitted.o: Sf_def_all_kinds.o Sg_0_fitted.f90
	$(f95) $(f95_FLAGS) Sg_0_fitted.f90

Sg_1_template_my_kind.o: Sg_0_fitted.o Sg_1_template_my_kind.f90
	$(f95) $(f95_FLAGS) Sg_1_template_my_kind.f90

Sg_2_template_my_kind.o: Sg_1_template_my_kind.o Sg_2_template_my_kind.f90
	$(f95) $(f95_FLAGS) Sg_2_template_my_kind.f90

Sh_def_kind.o: Sg_2_template_my_kind.o Sh_def_kind.f90
	$(f95) $(f95_FLAGS) Sh_def_kind.f90

Si_def_element.o: Sh_def_kind.o Si_def_element.f90
	$(f95) $(f95_FLAGS) Si_def_element.f90

Sj_elements.o: Si_def_element.o Sj_elements.f90
	$(f95) $(f95_FLAGS) Sj_elements.f90

Sk_link_list.o: Sj_elements.o Sk_link_list.f90
	$(f95) $(f95_FLAGS) Sk_link_list.f90

Sl_family.o: Sk_link_list.o Sl_family.f90
	$(f95) $(f95_FLAGS) Sl_family.f90

Sm_tracking.o: Sl_family.o Sm_tracking.f90
	$(f95) $(f95_FLAGS) Sm_tracking.f90

Sn_mad_like.o: Sm_tracking.o Sn_mad_like.f90
	$(f95) $(f95_FLAGS) Sn_mad_like.f90

So_fitting.o: Sn_mad_like.o So_fitting.f90
	$(f95) $(f95_FLAGS) So_fitting.f90

Sp_keywords.o: So_fitting.o Sp_keywords.f90
	$(f95) $(f95_FLAGS) Sp_keywords.f90

madx_ptc_module.o: Sp_keywords.o madx_ptc_module.f90
	$(f95) $(f95_FLAGS) madx_ptc_module.f90

wrap.o: madx_ptc_module.o wrap.f90
	$(f95) $(f95_FLAGS) wrap.f90

madx: \
	madxnp.o twissp.o ptc_dummy.o surveyp.o orbfp.o emitp.o utilp.o matchp.o matchsap.o dynapp.o \
	plotp.o ibsdbp.o trrunp.o gxx11p.o gxx11c.o
	$(FC) $(FP) -o madx madxm.F madxnp.o twissp.o ptc_dummy.o matchp.o matchsap.o ibsdbp.o plotp.o \
	trrunp.o dynapp.o surveyp.o orbfp.o emitp.o utilp.o gxx11p.o gxx11c.o $(LIBX) -lm -lc

madxdev: \
	a_scratch_size.o b_da_arrays_all.o c_dabnew.o d_lielib.o \
	e_define_newda.o f_newda.o g_newLielib.o h_definition.o \
	i_tpsa.o j_tpsalie.o k_tpsalie_analysis.o l_complex_taylor.o \
	m_real_polymorph.o n_complex_polymorph.o  \
	Sa_extend_poly.o Sb_1_pol_template.o Sb_2_pol_template.o \
	Sc_euclidean.o Sd_frame.o Se_status.o Sf_def_all_kinds.o Sg_0_fitted.o \
	Sg_1_template_my_kind.o Sg_2_template_my_kind.o Sh_def_kind.o \
	Si_def_element.o Sj_elements.o Sk_link_list.o Sl_family.o \
	Sm_tracking.o Sn_mad_like.o So_fitting.o Sp_keywords.o \
	madx_ptc_module.o wrap.o \
	madxm.o madxnp.o twiss.o survey.o orbf.o emit.o util.o 	match.o matchsa.o dynap.o \
	plot.o ibsdb.o trrun.o 	gxx11.o gxx11c.o epause.o timel.o usleep.o
	$(f95) $(FOPT) -o madxdev madxm.o \
	a_scratch_size.o b_da_arrays_all.o c_dabnew.o d_lielib.o \
	e_define_newda.o f_newda.o g_newLielib.o h_definition.o \
	i_tpsa.o j_tpsalie.o k_tpsalie_analysis.o l_complex_taylor.o \
	m_real_polymorph.o n_complex_polymorph.o  \
	Sa_extend_poly.o Sb_1_pol_template.o Sb_2_pol_template.o \
	Sc_euclidean.o Sd_frame.o Se_status.o Sf_def_all_kinds.o Sg_0_fitted.o \
	Sg_1_template_my_kind.o Sg_2_template_my_kind.o Sh_def_kind.o \
	Si_def_element.o Sj_elements.o Sk_link_list.o Sl_family.o \
	Sm_tracking.o Sn_mad_like.o So_fitting.o Sp_keywords.o \
	madx_ptc_module.o wrap.o \
	madxnp.o twiss.o survey.o orbf.o emit.o util.o 	match.o matchsa.o dynap.o \
	plot.o ibsdb.o trrun.o 	gxx11.o gxx11c.o epause.o timel.o usleep.o $(LIBX) -lm -lc

clean:
	rm -f *.o
	rm -f *.g90
	rm -f *.mod
	rm -f core
	rm -f *~
