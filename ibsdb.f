! **********************************************************
      subroutine enprem
      implicit none
!----------------------------------------------------------------------*
! Purpose:                                                             *
!   Print emittances and sigmas.                                       *
!----------------------------------------------------------------------*
!---- Double precision version.
!----------------------------------------------------------------------*
      double precision ten3p,ten6p
      parameter(ten6p=1d6,ten3p=1d3)
      include 'ibsdb.fi'

!---- Emittances and sigmas.
      write (*, 910) ten6p * ex, ten3p * sigx,                          &
     &ten6p * ey, ten3p * sigy,                                         &
     &ten6p * et, ten3p * sigt, ten3p * sige

  910 format(' '/' Emittances:'/' '/                                    &
     &t6,'Ex',t16,e16.6,' pi*mm*mrad',t48,'sigx',t58,f14.6,' mm'/       &
     &t6,'Ey',t16,e16.6,' pi*mm*mrad',t48,'sigy',t58,f14.6,' mm'/       &
     &t6,'Et',t16,e16.6,' pi*mm*mrad',t48,'sigt',t58,f14.6,' mm',       &
     &t88,'sigE',t96,f14.6,' 1/1000')

      end

! **********************************************************

      subroutine enprgl
      implicit none

#ifdef _WIN32
	include 'win32calls.fi'
#endif

!----------------------------------------------------------------------*
! Purpose:                                                             *
!   Print global data for machine.                                     *
!----------------------------------------------------------------------*
!---- Double precision version.
!----------------------------------------------------------------------*
      logical frad
      integer n1
      double precision eta,gamtr,t0,get_value,zero,one
      parameter(zero=0d0,one=1d0)
      include 'ibsdb.fi'

      n1 = get_value('probe ','radiate ')
      frad = n1.ne.0

!---- Global parameters.

      if (alfa .gt. zero) then
        gamtr = sqrt(one / alfa)
      else if (alfa .eq. zero) then
        gamtr = zero
      else
        gamtr = - sqrt(-one / alfa)
      endif
      t0 = one / freq0
      eta = alfa - one / gamma**2

      write (*, 910) frad, circ, freq0, t0, alfa,                       &
     &eta, gamtr, currnt, bunch, parnum, en0, gamma, beta



  910 format(' '/' Global parameters for the machine: ',//,             &
     &'radiate = ',l1,':'/' '/                                          &
     &t6,'C',t16,f14.6,' m',t46,'f0',t56,f14.6,' MHz',                  &
     &t86,'T0',t96,f14.6,' microseconds'/                               &
     &t6,'alfa',t16,e18.6,t46,'eta',t56,e18.6,                          &
     &t86,'gamma(tr)',t96,f14.6/                                        &
     &t6,'Bcurrent',t16,f14.6,' A/bunch',t46,'Kbunch',t56,I6,           &
     &t86,'Npart',t96,e18.6,' per bunch'/                               &
     &t6,'E',t16,f14.6,' GeV',t46,'gamma',t56,f14.6,                    &
     &t86,'beta',t96,f14.6)

      end
! *************************************************************
      subroutine cavprt()

      implicit none
#ifdef _WIN32
	include 'win32calls.fi'
#endif

      integer i,lg,code,get_string,restart_sequ,advance_node
      double precision get_value,node_value,el,rfv,rff,rfl,deltap
      include 'name_len.fi'
      character * (name_len) sequ_name,el_name

      lg = get_string('sequence ', 'name ', sequ_name)
      if (lg .gt. 0) print *, 'sequence name: ', sequ_name(:lg)
      i = restart_sequ()
   10 continue
      code = node_value('mad8_type ')
      if (code .eq. 10) then      ! cavity
      lg = get_string('element ', 'name ', el_name)
      el = node_value('l ')
      rfv = node_value('volt ')
      rff = node_value('freq ')
      rfl = node_value('lag ')
      deltap = get_value('probe ','deltap ')
      print '(a,5g14.6)', el_name(:lg), el, rfv, rff, rfl, deltap
      endif
      if (advance_node().ne.0)  goto 10
      end

! *********************************************************************
      subroutine twclog(bxbar, bybar, const)
      implicit none

#ifdef _WIN32
	include 'win32calls.fi'
#endif

!----------------------------------------------------------------------*
! Purpose:                                                             *
!   Calculation of Coulomb logarithm (and print)                       *
!   based on the formulae in AIP physics vade mecum p.264 (1981)       *
! Input:                                                               *
!   BXBAR     (real)    Average horizontal beta.                       *
!   BYBAR     (real)    Average vertical beta.                         *
! Output:                                                              *
!   CONST     (real)    Constant in eq. (IV.9.1), ZAP user's manual.   *
!----------------------------------------------------------------------*
!---- Double precision version.
!----------------------------------------------------------------------*
      logical fbch
      integer n
      double precision get_value,bgam,bxbar,bybar,cbunch,const,coulog,  &
     &debyel,densty,etrans,pnbtot,qion,rmax,rmin,rmincl,rminqm,sigtcm,  &
     &sigxcm,sigycm,tempev,vol,pi,get_variable,zero,two,four,eight,ot2, &
     &ft8,ot5,ttm3,fac1,fac2
      parameter(zero=0d0,two=2d0,four=4d0,eight=8d0,ot2=1d2,ft8=5d8,    &
     &ot5=1d5,ttm3=2d-3,fac1=743.4d0,fac2=1.44d-7)
      include 'ibsdb.fi'
      include 'physcons.fi'

      pi=get_variable('pi ')
! **************************** DB *********************
      n = get_value('probe ', 'bunched ')
      fbch = n.ne.0
! *****************************************************



!---- Calculate transverse temperature as 2*P*X',
!     i.e., assume the transverse energy is temperature/2.
      qion   = abs(charge)
      etrans = ft8 * (gammas * en0 - amass) * (ex / bxbar)
      tempev = two * etrans

!---- Calculate beam volume to get density (in cm**-3).
      sigxcm = ot2 * sqrt(ex * bxbar)
      sigycm = ot2 * sqrt(ey * bybar)
      sigtcm = ot2 * sigt
      if (fbch) then
      vol    = eight * sqrt(pi**3) * sigxcm * sigycm * sigtcm
      densty = parnum / vol
      else
      vol    = four * pi * sigxcm * sigycm * ot2 * circ
      pnbtot = currnt * circ / (qion * qelect * betas * clight)
      densty = pnbtot / vol
      endif

!---- Calculate RMAX as smaller of SIGXCM and DEBYE length.
      debyel = fac1 * sqrt(tempev/densty) / qion
      rmax   = min(sigxcm,debyel)

!---- Calculate RMIN as larger of classical distance of closest approach
!     or quantum mechanical diffraction limit from nuclear radius.
      rmincl = fac2 * qion**2 / tempev
      rminqm = hbar*clight*ot5 / (two*sqrt(ttm3*etrans*amass))
      rmin   = max(rmincl,rminqm)
      coulog = log(rmax/rmin)
      bgam = betas * gammas
      qion   = abs(charge)
      if (fbch) then
      const = parnum * coulog * arad**2 * clight / (eight * pi * betas  &
     &**3 * gammas**4 * ex * ey * sige * sigt)
      cbunch = qion * parnum * qelect * betas * clight / circ
      else
      const = currnt * coulog * arad**2 /                               &
     &(four * sqrt(pi) * qion * qelect * bgam**4 * ex * ey * sige)
      endif
      write (*, 910) const

      write (*, 920) en0, betas, gammas, coulog

!---- Print warning here if Coulomb logarithm gave bad results.
!     Usually this error is due to a starting guess far from
!     the equilibrium value.
      if (coulog .lt. zero) then
      call aawarn('TWCLOG', 'Coulomb logarithm gives invalid'           &
     &// ' result --- check input parameters.')
      endif

      write (*, 940) ex, ey

      if (fbch) then
      write (*, 950) sige, sigt, parnum, cbunch
      else
      write (*, 960) sige, currnt
      endif

  910 format(' '/5x,'CONST               = ',1p,e14.6)
  920 format(' '/5x,'ENERGY              = ',f14.6,' GeV'/              &
     &5x,'BETA                = ',f14.6/                                &
     &5x,'GAMMA               = ',f14.3/                                &
     &5x,'COULOMB LOG         = ',f14.3)
  940 format(' '/5x,'X-emittance         = ',1p,e14.6,' m*rad'/         &
     &5x,'Y-emittance         = ',   e14.6,' m*rad')
  950 format(' '/5x,'Momentum spread     = ',1p,e14.6/                  &
     &5x,'Bunch length        = ',0p,f14.6,' m'/' '/                    &
     &5x,'Particles per bunch = ',1p,e14.6/                             &
     &5x,'Bunch current       = ',1p,e14.6,' A')
  960 format(' '/5x,'Momentum spread     = ',1p,e14.6/' '/              &
     &5x,'Current             = ',0p,f14.6,' A'/' ')

      end
! *********************************************************************
      subroutine ibs

      implicit none

#ifdef _WIN32
        !DEC$ ATTRIBUTES C, ALIAS:'_ibs_' :: ibs
	include 'win32calls.fi'
#endif

!----------------------------------------------------------------------*
! Purpose:                                                             *
!   INTRABEAM SCATTERING, IBS Command                                  *
!   These routines are a much reduced version of IBS as taken          *
!   from the program ZAP, written by M. Zisman.                        *
!   One should refer to the ZAP USERS MANUAL LBL-21270 UC-28.          *
! Attribute:                                                           *
!   TABLE     (name)    Name of Twiss table.                           *
!----------------------------------------------------------------------*
      integer step,i,j,flag,range(2),n,get_option,double_from_table,    &
     &restart_sequ,advance_to_pos
      double precision tol,alx,alxbar,alxwtd,aly,alybar,ax1,ax2,ay1,ay2,&
     &betax,betay,beteff,bx1,bx2,bxbar,bxinv,by1,by2,bybar,byinv,bywtd, &
     &const,dels,dpx,dpx1,dpx2,dpxbr,dpxwtd,dx,dx1,dx2,dxbar,dxwtd,     &
     &hscrpt,hscwtd,s1,s2,salxb,salyb,sbxb,sbxinv,sbyb,sbyinv,sdpxb,    &
     &sdxb,taul,taux,tauy,tavl,tavlc,tavx,tavxc,tavy,tavyc,tlbar,tlidc, &
     &tlwtd,txbar,txidc,txwtd,tybar,tyidc,tywtd,wnorm,sdum,get_value,   &
     &get_variable,zero,one,two,half
      parameter(zero=0d0,one=1d0,two=2d0,half=0.5d0)
      include 'ibsdb.fi'
      include 'physcons.fi'

!---- Universal physical constants.

!     Permeability of vacuum [V*s/A*m]:
      amu0 = get_variable('amu0 ')
!     Permittivity of vaccum [A*S/V*m]:
      eps0 = get_variable('eps0 ')
!     Reduced Plack's constant [GeV*s]:
      hbar = get_variable('hbar ')

!---- Electromagnetic constants.
!     Elementary charge [A*s]:
      qelect = get_variable('qelect ')

!---- Electron.
!     Rest mass [GeV]:
      emass = get_variable('emass ')
!     Classical radius [m]:
      erad = get_variable('erad ')
!     Reduced Compton wavelength [m]:
      elamda = get_variable('elamda ')

!---- Proton.
!     Rest mass [GeV]:
      pmass = get_variable('pmass ')
!     Classical radius [m]:
      prad = get_variable('prad ')
!     Reduced Compton wavelength [m]:
      plamda = get_variable('plamda ')

!---- Muon.
!     Rest mass [GeV]:
      mumass = get_variable('mumass ')

! ************* Get the parameters for the common blocks *************
! *************         /machin/ and /beamdb/            *************


      charge   = get_value('probe ', 'charge ')
      gammas   = get_value('probe ', 'gamma ')
      gamma    = get_value('probe ', 'gamma ')
      en0      = get_value('probe ', 'energy ')
      amass    = get_value('probe ', 'mass ')
      ex       = get_value('probe ', 'ex ')
      ey       = get_value('probe ', 'ey ')
      et       = get_value('probe ', 'et ')
      sigt     = get_value('probe ', 'sigt ')
      sige     = get_value('probe ', 'sige ')
      parnum   = get_value('probe ', 'npart ')
      circ     = get_value('probe ', 'circ ')
      currnt   = get_value('probe ', 'bcurrent ')
      betas    = get_value('probe ', 'beta ')
      beta     = get_value('probe ', 'beta ')
      clight   = get_variable('clight ')
      arad     = get_value('probe ', 'arad ')
      alfa     = get_value('probe ', 'alfa ')
      freq0    = get_value('probe ', 'freq0 ')
      bunch   = get_value('probe ', 'kbunch ')

!  ****************** Test print ********
!     print *, 'Charge ', charge
!     print *, 'gammas ', gammas
!     print *, 'gamma ', gamma
!     print *, 'Energy ', en0
!     print *, 'Mass ', amass
!     print *, 'Ex ', ex
!     print *, 'Ey ', ey
!     print *, 'Et ', et
!     print *, 'sigt ', sigt
!     print *, 'sige ', sige
!     print *, 'parnum ', parnum
!     print *, 'circ ', circ
!     print *, 'currnt ', currnt
!     print *, 'betas ', betas
!     print *, 'beat ', beta
!     print *, 'clight ', clight
!     print *, 'arad ', arad
!     print *, 'alfa ', alfa
!     print *, 'freq0 ', freq0
!     print *, 'kbunch ', bunch
! ***************************************



!---- Initialize variables to accumulate weighted average lifetimes.
      tavlc  = zero
      tavxc  = zero
      tavyc  = zero
      dxwtd  = zero
      dpxwtd = zero
      bywtd  = zero
      alxwtd = zero
      hscwtd = zero
      wnorm  = zero
      sbxb   = zero
      sbyb   = zero
      salxb  = zero
      salyb  = zero
      sdxb   = zero
      sdpxb  = zero
      sbxinv = zero
      sbyinv = zero



! ****** Start new Twiss Table reading *****************
!
      step = get_value('ibs ', 'steps ')
      tol = get_value('ibs ', 'tolerance ')
!     print *, 'steps: ', step, '  tolerance: ', tol
      call table_range('twiss ', '#s/#e ', range)
!     print *, 'Range for Table ', range
      flag = double_from_table('twiss ', 's ', range(1), s1)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'betx ', range(1), bx1)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'bety ', range(1), by1)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'alfx ', range(1), ax1)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'alfy ', range(1), ay1)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'dx ', range(1), dx1)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'dpx ', range(1), dpx1)
      if (flag .ne. 0)  goto 102

! ************** Check if "ibs_table" required  ****************

      n = get_option('ibs_table ')

! ********** Start Do loop ***************
!
      j = restart_sequ()
      do i = range(1)+1, range(2)
      j = advance_to_pos('twiss ', i)
      flag = double_from_table('twiss ', 's ', i, s2)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'betx ', i, bx2)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'bety ', i, by2)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'alfx ', i, ax2)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'alfy ', i, ay2)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'dx ', i, dx2)
      if (flag .ne. 0)  goto 102
      flag = double_from_table('twiss ', 'dpx ', i, dpx2)
      if (flag .ne. 0)  goto 102
      dels = s2-s1

      sdum = half * (s2 + s1)

      betax  = half * (bx2 + bx1)
      sbxb   = sbxb + betax * dels
      sbxinv = sbxinv + dels / betax

      betay  = half * (by2 + by1)
      sbyb   = sbyb + betay * dels
      sbyinv = sbyinv + dels / betay

      alx    = half * (ax2 + ax1)
      salxb  = salxb + alx * dels

      aly    = half * (ay2 + ay1)
      salyb  = salyb + aly * dels

      dx     = half * (dx2 + dx1)
      sdxb   = sdxb + dx * dels

      dpx    = half * (dpx2 + dpx1)
      sdpxb  = sdpxb + dpx * dels


!*---- Calculate weighted average in region of non-zero DX's.
!     These values are used to calculate "average" ring lifetimes
!     in TWSINT.
      if (dx .gt. zero) then
        wnorm  = wnorm + dels
        dxwtd  = dxwtd + dels * dx
        dpxwtd = dpxwtd + dels * dpx
        bywtd  = bywtd + dels / sqrt(betay)
        alxwtd = alxwtd + dels * alx
        hscrpt = betax * dpx**2 + two * alx * dx * dpx +                &
     &  (one + alx**2) * dx**2 / betax
        hscwtd = hscwtd + dels * sqrt(hscrpt)
      endif

!---- TWSINT calculates the Bjorken/Mtingwa integral.
      call twsint(betax, betay, alx, dx, dpx, txidc, tyidc, tlidc)

!---- Accumulate contributions.
      tavlc = tavlc + tlidc * dels
      tavxc = tavxc + txidc * dels
      tavyc = tavyc + tyidc * dels

! *************** Fill "ibs_table" if required *********************

      if(n.ne.0) then
        call string_to_table('ibs ', 'name ', 'name ')
        call double_to_table('ibs ','s ', sdum)
        call double_to_table('ibs ','dels ', dels)
        call double_to_table('ibs ','tli ', tlidc)
        call double_to_table('ibs ','txi ', txidc)
        call double_to_table('ibs ','tyi ', tyidc)
        call augment_count('ibs ')
      endif

! *********** Make sure the following lines are not moved ******
! *********** by the compiler ***************
      s1   = s2
      bx1  = bx2
      by1  = by2
      ax1  = ax2
      ay1  = ay2
      dx1  = dx2
      dpx1 = dpx2

      enddo
      goto 101
  102 continue
      call aawarn('IBS ', 'table value not found, rest skipped ')
      stop
  101 continue

!---- We have finished reading the lattice from MAD
      bxbar  = sbxb / s2
      bybar  = sbyb / s2
      alxbar = salxb / s2
      alybar = salyb / s2
      dxbar  = sdxb / s2
      dpxbr  = sdpxb / s2
      bxinv  = sbxinv / s2
      byinv  = sbyinv / s2

      dxwtd  = dxwtd / wnorm
      dpxwtd = dpxwtd / wnorm
      bywtd  = bywtd / wnorm
      bywtd  = one / bywtd**2
      alxwtd = alxwtd / wnorm
      hscwtd = (hscwtd/wnorm)**2
      beteff = dxwtd**2 / hscwtd
! ********** Compute beam sizes with average betas ************

      sigx = sqrt(ex * bxbar)
      sigy = sqrt(ey * bybar)

      call enprgl
      call enprem
      call cavprt()
! ************************************************************

!---- Integral for averaged quantities.
      call twsint(bxbar,bybar,alxbar,dxbar,dpxbr, txbar,tybar,tlbar)

!---- Integral for effective quantities.
      call twsint(beteff,bywtd,alxwtd,dxwtd,dpxwtd, txwtd,tywtd,tlwtd)

!---- Calculate the Coulomb logarithm.
      call twclog(bxbar, bybar, const)

!---- Output (weighted) average values.
      write (*, 940) bxbar, bybar, dxbar, alxbar, alybar, dpxbr,        &
     &bxinv, byinv

!---- Output averaged values.
      tavl   = tavlc * const / s2
      tavx   = tavxc * const / s2
      tavy   = tavyc * const / s2

      taul   = one / tavl
      taux   = one / tavx
      tauy   = one / tavy

      write (*, 950) tavl, tavx, tavy, taul, taux, tauy

  910 format(' '/' Particle beam: ',a,10x,a,'bunched.')
  920 format(' '/' Individual lattice point lifetimes'/' '/             &
     &26x,'TLI/const',10x,'TXI/const',10x,'TYI/const'/                  &
     &27x,'(1/sec)',12x,'(1/sec)',12x,'(1/sec)'/' ')
  930 format(1x,i8,2x,a8,3x,3(1pe15.6,3x))
  940 format(' '/' Ring average values (m)'/' '/ 5x,'betx   = ',        &
     &1pe13.5,4x, 'bety   = ',1pe13.5,4x,'Dx  = ',1pe12.5/              &
     &5x,'alfx   = ',1pe13.5,4x,'alfy   = ',1pe13.5,4x,'Dpx = ',        &
     &1pe12.5/5x,'1/betx = ',1pe13.5,4x,'1/bety = ',1pe13.5)
  950 format(' '/5x,'(Weighted) average rates (1/sec):'/                &
     &5x,'Longitudinal= ',1p,e15.6/                                     &
     &5x,'Horizontal  = ',   e15.6/                                     &
     &5x,'Vertical    = ',   e15.6/                                     &
     &' '/5x,'(Weighted) average lifetimes (sec):'/                     &
     &5x,'Longitudinal= ',1p,e15.6/                                     &
     &5x,'Horizontal  = ',   e15.6/                                     &
     &5x,'Vertical    = ',   e15.6/' ')

      end
! *********************************************************************
      subroutine twsint(betax, betay, alx, dx, dpx, txi, tyi, tli)

      implicit none

#ifdef _WIN32
	include 'win32calls.fi'
#endif

!----------------------------------------------------------------------*
! Purpose:                                                             *
!   Subroutine uses Simpson's rule integration                         *
!   to calculate Bjorken/Mtingwa integrals (eqn. 3.4)                  *
!   Particle Accelerators 13, 115 (1983)                               *
!                                                                      *
!   Equivalent expressions are found in Conte/Martini                  *
!   Particle Accelerators 17, 1 (1985)                                 *
!                                                                      *
!   Integrals are broken into decades to optimize speed.               *
!                                                                      *
!   For the VAX, values may not exceed 10**33, therefore TSTLOG=33     *
!   For the IBM, values may not exceed 10**74, therefore TSTLOG=74     *
!   (PMG, March 1988)                                                  *
!                                                                      *
!   The integral is split into MAXDEC decades with NS steps /decade.   *
!   TEST is used for testing convergence of the integral               *
! Input:                                                               *
!   BETAX     (real)    Horizontal beta.                               *
!   BETAY     (real)    Vertical beta.                                 *
!   ALX       (real)    Horizontal alpha.                              *
!   DX        (real)    Horizontal dispersion.                         *
!   DPX       (real)    Derivative of horizontal dispersion.           *
! Output:                                                              *
!   TXI       (real)    Horizontal rate / const.                       *
!   TYI       (real)    Vertical rate / const.                         *
!   TLI       (real)    Longitudinal rate / const.                     *
!----------------------------------------------------------------------*
      integer iiz,iloop,maxdec,ns
      parameter(maxdec=30,ns=50)
      double precision a,al(31),alam,aloop,alx,am,b,betax,betay,bl(30), &
     &c1,c2,c3,ccy,chklog,cl,coeff(2),cof,cprime,cscale,cx,cy,dpx,dx,f, &
     &func,h,phi,polyl,polyx,polyy,r1,suml,sumx,sumy,td1,td2,term,tl1,  &
     &tl2,tli,tmpl,tmpx,tmpy,tx1,tx2,txi,ty1,ty2,tyi,zintl,zintx,zinty, &
     &zero,one,two,three,six,tstlog,power,ten,test
      parameter(zero=0d0,one=1d0,two=2d0,three=3d0,six=6d0,tstlog=74d0, &
     &power=-two/three,ten=1d1,test=1d-7)
      include 'ibsdb.fi'
      include 'physcons.fi'
      data coeff / 2d0, 4d0 /

      phi    = dpx + (alx * dx / betax)
      am     = one
      c1     = (gammas * dx)**2 / (ex * betax)
      c3     = betax / ex
      c2     = c3 * (gammas*phi)**2
      cx     = c1 + c2
      cl     = am * (gammas/sige)**2
      cy     = betay / ey
      r1     = three / cy
      a      = cx + cl
      b      = (c3 + cy) * (c1 + cl) + cy * c2

!---- Define CPRIME=C*CSCALE to try to keep the value.
!     small enough for the VAX in single precision or
!     IBM in double precision.
!     Test LOG(C) to see if it needs scaling
      cscale = one
      chklog = log10(c3) + log10(cy) + log10(c1 + cl)
      if (chklog .gt. tstlog) cscale = ten**(tstlog-chklog)
      cprime = c3 * cy * cscale * (c1 + cl)

!---- Split integral into decades, with NS steps per decade.
!     variables to save integral segments
      zintl  = zero
      zintx  = zero
      zinty  = zero

!---- Constants for integration loop.
!     To keep the numbers reasonable, the numerator is
!     scaled by 1/CPRIME and the denominator by 1/CPRIME**2.
!     The extra factor of CPRIME is accounted for after integrating
      ccy    = cprime**power
      td1    = (a + c3) * ccy
      td2    = one / (sqrt(ccy) * cscale * cy)
      tl1    = (two * a - cy - c3) / cprime
      tl2    = (b - two * c3 * cy) / cprime
      ty1    = (- a - c3 + two * cy) / cprime
      ty2    = (b + cy * c3) / cprime - r1 / cscale
      tx1    = (two * a * (cx - c3) - cy * cx -                         &
     &c3 * (cy - cl - two * c3 - six * c2)) / cprime
      tx2    = (c3 + cx) * ((b + c3 * cy) / cprime) -                   &
     &six / cscale + three * c3 * cy * (cl / cprime)

      al(1)  = zero

      do iloop = 1, maxdec
      bl(iloop) = ten**iloop
      al(iloop+1) = bl(iloop)
      h = (bl(iloop) - al(iloop)) / ns
      aloop = al(iloop)

!---- Evaluate Simpson's rule summation for one interval.
!     The integrand is calculated in the loop itself
      term = sqrt((cy + aloop) * ccy) *                                 &
     &sqrt(aloop * ccy * aloop + td1 * aloop + td2)
      func = sqrt(aloop) / term**3
      polyl = tl1 * aloop + tl2
      polyx = tx1 * aloop + tx2
      polyy = ty1 * aloop + ty2
      suml = func * polyl
      sumx = func * polyx
      sumy = func * polyy


      do iiz = 1, ns
        alam = aloop + iiz * h
        cof = coeff(mod(iiz,2)+1)
        term = sqrt((cy+alam)*ccy) *                                    &
     &  sqrt(alam*ccy*alam+td1*alam+td2)
        f = sqrt(alam) / term**3
        polyl = tl1 * alam + tl2
        polyx = tx1 * alam + tx2
        polyy = ty1 * alam + ty2

        suml = suml + cof * f * polyl
        sumx = sumx + cof * f * polyx
        sumy = sumy + cof * f * polyy
      enddo

      suml = suml - f * polyl
      sumx = sumx - f * polyx
      sumy = sumy - f * polyy
      tmpl = (suml / three) * h
      tmpx = (sumx / three) * h
      tmpy = (sumy / three) * h
      zintl = zintl + tmpl
      zintx = zintx + tmpx
      zinty = zinty + tmpy

!---- Test to see if integral has converged.
      if (abs(tmpl/zintl) .lt. test .and.                               &
     &abs(tmpx/zintx) .lt. test .and.                                   &
     &abs(tmpy/zinty) .lt. test) goto 100
      enddo
      write (*, 910) maxdec
  910 format('Bjorken/Mtingwa integrals did not converge in ',          &
     &i3,' decades.')
      call aawarn('TWSINT: ', 'Problem with TWSINT, program stopped ')
      stop
  100 continue

!---- Divide answers by cprime to account for scaling.
      txi    =      (zintx / cprime)
      tli    = cl * (zintl / cprime)
      tyi    = cy * (zinty / cprime)

      end
! ***************************************************************
