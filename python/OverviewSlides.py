#!/usr/bin/env python

"""
Create an overview pdf-presentation of multiple runs
"""

# Flux in kHz
# (taken from Steve's Spreadsheet)
di_runs = {
    322 : 1.6,
    325 : 12.9,
    327 : 130.6,
    330 : 1167,
    333 : 20809,
    338 : 1137,
    340 : 125.4,
    343 : 10.8,
    347 : 1.4,
    348 : 1.5,
    350 : 20809.2,
    352 : 21387.3,
    }

roc_names={
  1:"Poly A",
  2:"Poly D",
  3:"s86",
  4:"s105",
}
roc_name1 = "Poly A"
roc_name2 = "Poly D"
roc_name3 = "s86"
roc_name4 = "s105"

latex_preamble = r"""
\documentclass{beamer}

\usepackage{graphicx}
\usepackage{float}
\usepackage[caption = false]{subfig}

\setbeamerfont{page number in head/foot}{size=\large}
\setbeamertemplate{footline}[frame number]
\setbeamertemplate{navigation symbols}{}

\renewcommand{\thesubfigure}{}

\begin{document}

  \title{Plots for runs 322 -- 352}
  \author{GK}
  \date{17. June 2014}


  \frame{\titlepage}
 """

latex_finnish = r"""\end{document}
"""

def newframe( title, file ):
  s = r"""\begin{frame}
\frametitle{XXX}
""".replace("XXX", title)
  file.write(s)

def endframe( file ):
  file.write( r"\end{frame}" + "\n" )

def section( name, file):
  s=r"""\begin{frame}[plain,c]
  \begin{center}
  \Huge XXX
  \end{center}
  \end{frame}""".replace( "XXX", name)
  f.write(s)


def overview( run, roc, file):
  s = r"""\begin{figure}
          \subfloat[$\varepsilon$]{\includegraphics[width = .35\textwidth]{../plots/000RUN/PlaneEfficiency_NROC.pdf}}
          \subfloat[R $<$ 0.01 cm]{\includegraphics[width = .35\textwidth]{../plots/000RUN/Charge_NROC_profile.pdf}}
          \subfloat[R $<$ 0.02 cm]{\includegraphics[width = .35\textwidth]{../plots/000RUN/Charge02_NROC_profile.pdf}} \\
          \subfloat[1D]{\includegraphics[width = .35\textwidth]{../plots/000RUN/Charge_NROC.pdf}}
          \subfloat[R $<$ 0.03 cm]{\includegraphics[width = .35\textwidth]{../plots/000RUN/Charge03_NROC_profile.pdf}}
          \subfloat[R $<$ 0.04 cm]{\includegraphics[width = .35\textwidth]{../plots/000RUN/Charge04_NROC_profile.pdf}}
          \end{figure}
          """
  s = s.replace("NROC", "ROC"+str(roc))
  s = s.replace("RUN", str(run))
  f.write(s)


def effs(run, file):
  s = r"""\begin{figure}
          \subfloat[NROC1]{\includegraphics[width = .45\textwidth]{../plots/000RUN/PlaneEfficiency_ROC1.pdf}}
          \subfloat[NROC2]{\includegraphics[width = .45\textwidth]{../plots/000RUN/PlaneEfficiency_ROC2.pdf}}\\
          \subfloat[NROC3]{\includegraphics[width = .45\textwidth]{../plots/000RUN/PlaneEfficiency_ROC3.pdf}}
          \subfloat[NROC4]{\includegraphics[width = .45\textwidth]{../plots/000RUN/PlaneEfficiency_ROC4.pdf}}
          \end{figure}
          """
  s = s.replace("NROC1", roc_names[1])
  s = s.replace("NROC2", roc_names[2])
  s = s.replace("NROC3", roc_names[3])
  s = s.replace("NROC4", roc_names[4])
  s = s.replace("RUN", str(run))
  f.write(s)


def trans1d(run, file):
  s = r"""\begin{figure}
          \subfloat[NROC1]{\includegraphics[width = .45\textwidth]{../plots/000RUN/Charge_ROC1.pdf}}
          \subfloat[NROC2]{\includegraphics[width = .45\textwidth]{../plots/000RUN/Charge_ROC2.pdf}}\\
          \subfloat[NROC3]{\includegraphics[width = .45\textwidth]{../plots/000RUN/Charge_ROC3.pdf}}
          \subfloat[NROC4]{\includegraphics[width = .45\textwidth]{../plots/000RUN/Charge_ROC4.pdf}}
          \end{figure}
          """

  s = s.replace("NROC1", roc_names[1])
  s = s.replace("NROC2", roc_names[2])
  s = s.replace("NROC3", roc_names[3])
  s = s.replace("NROC4", roc_names[4])
  s = s.replace("RUN", str(run))
  f.write(s)

def trans2d(run, radius, file):
  s = r"""\begin{figure}
          \subfloat[NROC1]{\includegraphics[width = .45\textwidth]{../plots/000RUN/ChargeRRR_ROC1_profile.pdf}}
          \subfloat[NROC2]{\includegraphics[width = .45\textwidth]{../plots/000RUN/ChargeRRR_ROC2_profile.pdf}}\\
          \subfloat[NROC3]{\includegraphics[width = .45\textwidth]{../plots/000RUN/ChargeRRR_ROC3_profile.pdf}}
          \subfloat[NROC4]{\includegraphics[width = .45\textwidth]{../plots/000RUN/ChargeRRR_ROC4_profile.pdf}}
          \end{figure}
          """
  s = s.replace("NROC1", roc_names[1])
  s = s.replace("NROC2", roc_names[2])
  s = s.replace("NROC3", roc_names[3])
  s = s.replace("NROC4", roc_names[4])
  s = s.replace("RUN", str(run))

  if radius == 1:
    s=s.replace("RRR","")
  elif radius == 2:
    s=s.replace("RRR","02")
  elif radius == 3:
    s=s.replace("RRR","03")
  elif radius == 4:
    s=s.replace("RRR","04")

  f.write(s)



f = open ("Overview.tex","w")
f.write( latex_preamble)

# # Efficiencies
# section("Efficiencies", f)
# for run in sorted(di_runs):
#   newframe( "Efficiency, Run {0:d}, Flux={1:3.1f} kHz".format(run, di_runs[run]), f)
#   effs(run,f)
#   endframe(f)
#
#
# # Transparent, 1D
# section("Transparent Analysis", f)
# for run in sorted(di_runs):
#   newframe( "Transparent, Run {0:d}, Flux={1:3.1f} kHz".format(run, di_runs[run]), f)
#   trans1d(run,f)
#   endframe(f)
#
# # Transparent
# section(r"Transparent Analysis, Charge within R$<$0.01cm", f)
# for run in sorted(di_runs):
#   newframe( r"Transparent R$<$0.01, " +"Run {0:d}, Flux={1:3.1f} kHz".format(run, di_runs[run]), f)
#   trans2d(run,1,f)
#   endframe(f)
#
# # Transparent
# section(r"Transparent Analysis, Charge within R$<$0.02cm", f)
# for run in sorted(di_runs):
#   newframe( r"Transparent R$<$0.02, " +"Run {0:d}, Flux={1:3.1f} kHz".format(run, di_runs[run]), f)
#   trans2d(run,2,f)
#   endframe(f)
#
# # Transparent
# section(r"Transparent Analysis, Charge within R$<$0.03cm", f)
# for run in sorted(di_runs):
#   newframe( r"Transparent R$<$0.03, " +"Run {0:d}, Flux={1:3.1f} kHz".format(run, di_runs[run]), f)
#   trans2d(run,3,f)
#   endframe(f)
#
# # Transparent
# section(r"Transparent Analysis, Charge within R$<$0.04cm", f)
# for run in sorted(di_runs):
#   newframe( r"Transparent R$<$0.04, " +"Run {0:d}, Flux={1:3.1f} kHz".format(run, di_runs[run]), f)
#   trans2d(run,4,f)
#   endframe(f)


for roc in [1,2,3,4]:
  section( roc_names[roc], f)
  for run in sorted(di_runs):
    newframe( roc_names[roc]+ ", Run {0:d}, Flux={1:3.1f} kHz".format(run, di_runs[run]), f)
    overview(run,roc,f)
    endframe(f)





f.write(latex_finnish)
