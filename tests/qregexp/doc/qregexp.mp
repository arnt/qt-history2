% qregexp.mp

prologues := 0;
input boxes;

vardef tr(suffix a,b) expr p =
  drawarrow p cutbefore bpath.a cutafter bpath.b;
  point 0.5*length p of p
enddef;

def state(suffix a,b)(text t) =
  circleit.a();
  label(t)(z.b);
  a.c = z.b;
enddef;

beginfig(1) % automaton for (?:ab|a)*
  interim circmargin := 8bp;

  z0 = (0,0);
  z4 - z0 = z1 - z4
    = z3 - z2
    = (z2 - z0) rotated -60
    = (72bp,0);
  z5 = z0 - (circmargin,0);
  (z5 - z6) rotated 45
    = (z5 - z7) rotated -45
    = (6bp, 0);

  state(a)(0)("0");
  state(b)(1)("1");
  state(c)(2)("2");
  state(d)(3)("3");
  state(e)(4)("4");

  drawboxed(a,b,c,d,e);

  draw z6--z5--z7;

  interim circmargin := 10bp;
  circleit.bb(pic b);
  bb.c = z1;
  drawboxed(bb);

  label.bot(btex $\epsilon$ etex,tr(a,bb) a.c{dir-50}..bb.c);
  label.ulft(btex $a$ etex,tr(a,c) a.c--c.c);
  label.top(btex $a$ etex,tr(a,e) a.c--e.c);
  label.top(btex $b$ etex,tr(c,d) c.c--d.c);
  label.urt(btex $\epsilon$ etex,tr(d,bb) d.c--bb.c);
  label.top(btex $a$ etex,tr(d,c) d.c..{dir-120}c.c);
  label.ulft(btex $a$ etex,tr(d,e) d.c--e.c);
  label.urt(btex $a$ etex,tr(e,c) e.c--c.c);
  label.top(btex $\epsilon$ etex,tr(e,bb) e.c--bb.c);
  label.bot(btex $a$ etex,tr(e,e) e.c{dir-150}..e.c-(0,17bp)..{dir150}e.c);
endfig;

beginfig(2) % automaton for (?:a*$b+)+
  interim circmargin := 8bp;

  z0 = (0,0);
  z2 - z0 = z1 - z3
    = (z3 - z0) rotated 90
    = (72bp,0);
  z4 = z0 - (circmargin,0);
  (z4 - z5) rotated 45
    = (z4 - z6) rotated -45
    = (6bp,0);

  state(a)(0)("0");
  state(b)(1)("1");
  state(c)(2)("2");
  state(d)(3)("3");

  drawboxed(a,b,c,d);

  draw z5--z4--z6;

  interim circmargin := 10bp;
  circleit.bb(pic b);
  bb.c = z1;
  drawboxed(bb);

  label.top(btex $a$ etex,tr(a,c) a.c--c.c);
  label.lft(btex \$ $b$ etex,tr(a,d) a.c--d.c);
  label.ulft(btex \$ $b$ etex,tr(c,d) c.c+(0,4bp)--d.c+(0,4bp));
  label.top(btex $\epsilon$ etex,tr(d,bb) d.c--bb.c);
  label.lrt(btex $a$ etex,tr(d,c) d.c-(0,4bp)--c.c-(0,4bp));
  label.bot(btex $b$ etex,tr(d,d) d.c{dir-150}..d.c-(0,17bp)..{dir150}d.c);
endfig;

beginfig(3) % automaton for (a*)\1
  interim circmargin := 8bp;

  z0 = (0,0);
  z3 - z0 = z1 - z3
    = (z3 - z2) rotated 90
    = (z4 - z3) rotated 90
    = (72bp,0);
  z5 = z0 - (circmargin,0);
  (z5 - z6) rotated 45
    = (z5 - z7) rotated -45
    = (6bp,0);

  state(a)(0)("0");
  state(b)(1)("1");
  state(c)(2)("2");
  state(d)(3)("3");
  state(e)(4)("4");

  drawboxed(a,b,c,d,e);

  draw z6--z5--z7;

  interim circmargin := 10bp;
  circleit.bb(pic b);
  bb.c = z1;
  drawboxed(bb);

  label.ulft(btex $a$ etex,tr(a,c) a.c--c.c);
  label.top(btex $a$ etex,tr(c,c) c.c{dir150}..c.c+(0,17bp)..{dir-150}c.c);
  label.top(btex $b$ etex,tr(a,d) a.c--d.c);
  label.lft(btex $b$ etex,tr(c,d) c.c--d.c);
  label.top(btex $(\backslash1 = \epsilon)\ \epsilon$ etex,tr(d,bb) d.c--bb.c);
  label.lft(btex $\backslash1$ etex,tr(d,e) d.c--e.c);
  label.lrt(btex $\epsilon$ etex,tr(e,bb) e.c--bb.c);
endfig;

end;
