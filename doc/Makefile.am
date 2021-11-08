## Process this file with automake to produce Makefile.in

SUBDIRS = info schema

pdf = dotguide.pdf dottyguide.pdf neatoguide.pdf libgraph/Agraph.pdf libguide/libguide.pdf
html = build.html FAQ.html index.html winbuild.html pspdf.png char.html gdtclft.entities.example.png todo.html internal_todo.html
txt = Dot.ref latex_suggestions.txt fontfaq.txt addingLayout.txt

if WITH_LEFTY
pdf += leftyguide.pdf
endif

if WITH_SMYRNA
pdf += smyrna/smyrna.pdf
endif

pdfdir = $(pkgdatadir)/doc/pdf
htmldir = $(pkgdatadir)/doc/html
txtdir = $(pkgdatadir)/doc

pdf_DATA = $(pdf)
html_DATA = $(html)
txt_DATA = $(txt)

EXTRA_DIST = $(txt) $(pdf) $(html) schema/*.xml schema/*.xslt infosrc/*
