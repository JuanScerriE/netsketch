% !TeX program = lualatex

%% TODO: Update Bibiliography related commands

%% Inspiration: University of Malta - LaTeX Template for Thesis/Dissertation/FYP
%% by Proj. Jean-Paul Ebejer

\NeedsTeXFormat{LaTeX2e}

\ProvidesClass{uom-coursework}[2024/03/09 v0.1 University of Malta Coursework]

\newcommand{\@makeumpagestyle}{
\makepagestyle{umpage}

%% This travesty is due to a "bug" (well it might just be intended behaviour) in memoir, see
%% https://tex.stackexchange.com/questions/468922/oneside-in-memoir-causing-header-trouble

%% Header text
\makeevenhead{umpage}{\color{gray}\sffamily\small\leftmark}{}{\color{gray}\sffamily\small\rightmark}
\makeoddhead{umpage}{\color{gray}\sffamily\small\leftmark}{}{\color{gray}\sffamily\small\rightmark}

%% Foot text
\makeevenfoot{umpage}{}{\thepage}{} % Page number at bottom
\makeoddfoot{umpage}{}{\thepage}{}

%% Header and Footer rules
\makeheadfootruleprefix{umpage}{\color{gray}}{}
\makeheadrule{umpage}{\textwidth}{\normalrulethickness}

\makepsmarks{umpage}{%
	\createmark{chapter}{left}{shownumber}{\@chapapp\ }{. \ }
	\createmark{section}{right}{shownumber}{}{. \ }
}

}

%% Allow for manual configuration of the memoir class
\DeclareOption{article}{
\renewcommand{\@makeumpagestyle}{
\makepagestyle{umpage}

%% Header text
\makeevenhead{umpage}{}{}{\color{gray}\sffamily\small\rightmark}
\makeoddhead{umpage}{}{}{\color{gray}\sffamily\small\rightmark}

%% Foot text
\makeevenfoot{umpage}{}{\thepage}{} % Page number at bottom
\makeoddfoot{umpage}{}{\thepage}{}

%% Header and Footer rules
\makeheadfootruleprefix{umpage}{\color{gray}}{}
\makeheadrule{umpage}{\textwidth}{\normalrulethickness}

\makepsmarks{umpage}{%
	\createmark{section}{right}{shownumber}{}{. \ }
}

}

\PassOptionsToClass{\CurrentOption}{memoir}
}

\DeclareOption*{
\PassOptionsToClass{\CurrentOption}{memoir}
}

\ProcessOptions\relax


%% Load the class with these default options
\LoadClass[12pt,a4paper,oneside,final]{memoir}

%% **** Packages (Start) ****

\RequirePackage{iftex}

\RequireLuaHBTeX

\RequirePackage{polyglossia} % Load polyglossia to help take care of language-specific typesetting conventions and hyphenation
\RequirePackage{xcolor} % For named colours
\RequirePackage{eso-pic} % For more interesting and powerful ways of annotating with images
\RequirePackage{graphicx} % For pictures
\RequirePackage{amssymb} % At least, for black squares in bullet list
\RequirePackage{amsthm}	% For theorems, proof, lemmas, conjectures etc
\RequirePackage{amsmath} % Extension of maths mode in LaTeX etc
\RequirePackage{hyperref} % For hyper references
\RequirePackage{longtable} % For very long tables
\RequirePackage{listings} % For code listings
\RequirePackage{booktabs} % For more professional looking tables
\RequirePackage{tikz} % For creating painful but beautiful SVG graphics
\RequirePackage{coffeestains} % No reading is truly ever starts without an accompanying cup of coffee (or ... tea)
\RequirePackage{tcolorbox} % If we don't have fancy boxes then what are we doing?
\RequirePackage{fontspec} % This gives us more granular control over our fonts
\RequirePackage{mathtools} % Extension of amsmath
\RequirePackage{datetime} % For Current Month and Year (cover page)

%% The below is a list of packages with some pre-set options
\RequirePackage[printonlyused,withpage]{acronym}

%% These packages are related to citing
%\RequirePackage[backend=biber]{biblatex}
%\RequirePackage{apacite}
%\RequirePackage[authoryear,semicolon,sort]{natbib} % for (Ebejer, 2012; Alba, 2013) kind of references; removed 'square' option after viva examination comments

%% The below is a list of packages that might be useful to add
%% in the future but currently they seem to be overkill to
%% add to a class
%\RequirePackage{titlesec, titletoc} % Improved control over List of Contents, List of Tables etc
%\RequirePackage{adjustbox} % Allow for the manipulation of many more text containing objects as graphics
%\RequirePackage[ruled]{algorithm2e} % For writing algorithms in a very mathematical pseudocode
%\RequirePackage{subcaption} % Enhanced support for subfigures and subtables within a single figure or table environment
%\RequirePackage{bytefield} % For typesetting bit and byte fields in various contexts such as network protocols, data structures etc
%\RequirePackage{ragged2e} % An improved \raggedright (\RaggedRight)
%\RequirePackage{cancel} % For writing maths cancellations
%\RequirePackage{float} % Improve control over floating objects
%\RequirePackage[siunitx]{circuitikz} % Adds a bunch of circuits related Tikz objects
%\RequirePackage{pgfornament} % Allows you to use a bunch of ornaments
%\RequirePackage[shortlabels]{enumitem} % Allow for more control of enumeration environments
%\RequirePackage{dirtree} % If you need to create a tree directory graphic
%\RequirePackage{floatpag} % Used to hide page number on pages which contain only floats ... put a \thisfloatpagestyle{empty} in the float env, e.g. figure ...
%\RequirePackage{pdflscape} % For landscape tables (instead of portrait)
%\RequirePackage[width=.95\textwidth]{caption} % Set the width of the captions
%\RequirePackage[showframe]{geometry} % Enable a wireframe for visibility
% \RequirePackage{unicode-math} % Allow more fancy Math font manipulation

%% **** Packages (End) ****

%% **** Definitions & New Commands (Start) ****

%% The below is a list of mathematical operations which
%% for some reason are not defined
\newcommand*{\floor}[1]{\left\lfloor #1 \right\rfloor}
\newcommand*{\ceil}[1]{\left\lceil #1 \right\rceil}
\newcommand*{\round}[1]{\left\lfloor #1 \right\rceil}
\newcommand*{\abs}[1]{\left\lvert #1 \right\rvert}

%% The following is a list of University of Malta's official
%% colour palette
\definecolor{UMP200}{RGB}{186,12,47}
\definecolor{UMPBlk}{RGB}{0,0,0}
\definecolor{UMP872}{RGB}{133,113,77}
\colorlet{UMRed}{UMP200}
\colorlet{UMBlack}{UMPBlk}
\colorlet{UMBrown}{UMP872}
\definecolor{UMc1}{RGB}{255,237,0}
\definecolor{UMc2}{RGB}{253,196,45}
\definecolor{UMc3}{RGB}{247,164,0}
\definecolor{UMc4}{RGB}{237,108,5}
\definecolor{UMc5}{RGB}{255,52,33}
\definecolor{UMc6}{RGB}{156,12,53}
\definecolor{UMc7}{RGB}{233,76,94}
\definecolor{UMc8}{RGB}{240,133,144}
\definecolor{UMc9}{RGB}{245,180,199}
\definecolor{UMc10}{RGB}{250,193,138}
\definecolor{UMc11}{RGB}{195,179,216}
\definecolor{UMc12}{RGB}{144,32,130}
\definecolor{UMc13}{RGB}{29,49,118}
\definecolor{UMc14}{RGB}{0,79,159}
\definecolor{UMc15}{RGB}{0,117,190}
\definecolor{UMc16}{RGB}{103,185,232}
\definecolor{UMc17}{RGB}{143,160,190}
\definecolor{UMc18}{RGB}{102,193,191}
\definecolor{UMc19}{RGB}{0,152,120}
\definecolor{UMc20}{RGB}{174,178,25}
\definecolor{UMc21}{RGB}{101,179,46}
\definecolor{UMc22}{RGB}{0,88,71}
\definecolor{UMc23}{RGB}{179,106,31}
\definecolor{UMc24}{RGB}{137,76,83}

%% The year and term the coursework is submitted
\def\courseworkdate#1{\gdef\@courseworkdate{#1}}
%% The full (unabbreviated) name of the coursework
\def\courseworkname#1{\gdef\@courseworkname{#1}}
%% The name of your department (e.g. Computer Science, Statistics, Biochemistry, AI)
\def\department#1{\gdef\@department{#1}}
%% The name of your faculty
\def\faculty#1{\gdef\@faculty{#1}}
%% The subject code
\def\subjectcode#1{\gdef\@subjectcode{#1}}
%% The tagline
\def\tagline#1{\gdef\@tagline{#1}}
%% The document type, e.g. a dissertation or a thesis
\def\doctype#1{\gdef\@doctype{#1}}
%% The ID of the author
\def\authorid#1{\gdef\@authorid{#1}}


%% **** UM Definitions (End) ****

%% **** Document Options (Start) ****

%% Define the colors we are going to use for code listings
\colorlet{codegreen}{UMc21}
\colorlet{codegray}{UMc17}
\colorlet{codemagenta}{UMc8}
\colorlet{codepurple}{UMc12}
\definecolor{backcolor}{RGB}{238,238,238}

\lstdefinestyle{code}{%
	basicstyle=\ttfamily\small,
	commentstyle=\color{codegray}\itshape,
	keywordstyle=\color{codepurple},
	stringstyle=\color{codegreen},
	aboveskip=15pt,
	captionpos=b,
	abovecaptionskip=12.5pt,
	breaklines=true,
	numbers=none,
	frame=tb,
	framesep=5pt,
	keepspaces=true,
	showspaces=false,
	showstringspaces=false,
	breakatwhitespace=false,
	tabsize=2,
	showtabs=false,
}

\lstset{style=code}

%\OnehalfSpacing % One and a half line spacing

\setlength{\headsep}{1.5cm} % Add space between the header and text

\nouppercaseheads % Do not convert titles to uppercase

\setlength{\parindent}{0em} % Remove the paragraph indent

\setlength{\parskip}{0.75em} % Instead increase the spacing between each paragraph

\@makeumpagestyle

%% The backmatter is usually used for the bibliography
\copypagestyle{umpageback}{umpage}
\makeevenhead{umpageback}{\color{gray}\sffamily\small\leftmark}{}{}
\makeoddhead{umpageback}{}{}{\color{gray}\sffamily\small\rightmark}

\setlrmarginsandblock{3.7cm}{2.5cm}{*} % Set the page margins (for one and two sided docs)
\checkandfixthelayout % Put layout into effect

\graphicspath{{./images/}} % Where to look for images (paths) ...

\DeclareGraphicsExtensions{.pdf,.jpeg,.png,.jpg} % Graphics extensions to load

\newdateformat{monthyeardate}{\monthname[\THEMONTH], \THEYEAR} % Our date format

%% other possible values: bianchi, bringhurst, brotherton,
%% chappell, crosshead, culver, dash, demo2, demo3, dowding, ell (default),
%% ger, komalike, lyhne, madsen, ntglike, pedersen, southall,
%% tandh, thatcher, veelo, verville, wilsondob

\usetikzlibrary{calc}

%% Attribution: Constantina & daleif, https://tex.stackexchange.com/questions/593978/how-to-modify-chapter-style-in-memoir

\newif\ifNoChapNumber

\makechapterstyle{rigid}{
	\renewcommand\chapternamenum{} % erase default style
	\renewcommand\printchaptername{} % erase default style
	\renewcommand\printchapternum{} % erase default style
	\renewcommand\chapnumfont{\Large\sffamily}
	\renewcommand\chaptitlefont{\LARGE\bfseries\sffamily}
	\renewcommand\printchaptertitle[1]{%
		%\hfill % uncomment if you wish for right indented chapters
		\begin{tikzpicture}
			\node[inner sep=0pt,anchor=north west] (title) at (0,0)
			{\raggedright\chaptitlefont ##1};
			\coordinate (decobot) at (title.south west);
			\coordinate (decotop) at (title.north west);
			\ifNoChapNumber\else
			\node[inner sep=0pt,anchor=south west] (chapname) at
			($(title.north west)+(0,0.2)$) {\chapnumfont\@chapapp~\thechapter};
			\coordinate (decotop) at (chapname.north west);
			\fi
			\def\DecoAdj{0.2}
			\coordinate (decotop) at ($(decotop)+(-\DecoAdj,\DecoAdj)$);
			\coordinate (decobot) at ($(decobot)+(-\DecoAdj,-\DecoAdj)$);
			\fill[UMRed] (decotop) rectangle ++(-\DecoAdj,-\DecoAdj);
			\fill[UMRed] (decotop) rectangle ++(\DecoAdj,\DecoAdj);
			\fill[UMRed] (decobot) rectangle ++(\DecoAdj,-\DecoAdj);
			\fill[UMRed] (decobot) rectangle ++(-\DecoAdj,\DecoAdj);
		\end{tikzpicture}
	}
	\renewcommand\printchapternonum{\NoChapNumbertrue}
}

\chapterstyle{rigid}

\setsecheadstyle{\sffamily\LARGE\color{UMRed}} % Style head of sections
\setsubsecheadstyle{\sffamily\Large\color{UMRed}} % Style head of sub-sections
\setsubsubsecheadstyle{\sffamily\large\color{UMRed}} % Style head of sub-sub-sections

\setsecnumformat{\csname the#1\endcsname\enskip{\color{gray}\textbar}\enskip} % Style the section numbering format
\setsecnumdepth{subsection} % Three level depth - chapter, section, subsection. Subsubsections not included.

\newsubfloat{figure} % Declares a new subfloat element which allows to use \subfigure and \subbottom commands (part of subfig package)
\feetbelowfloat % We want the footnotes below floats, not strangely above

%% recolour the default itemize symbols to UMRed
\renewcommand{\labelitemi}{\color{UMRed}\textbullet}
\renewcommand{\labelitemii}{\color{UMRed}\textendash}
\renewcommand{\labelitemiii}{\color{UMRed}\textasteriskcentered}
\renewcommand{\labelitemiv}{\color{UMRed}\textperiodcentered}

%% TODO: Figure out why this was here and what is the actual
%% difference
% \newcommand{\ra}[1]{\renewcommand{\arraystretch}{#1}} % to set array stretch within tables

\hypersetup{%
	colorlinks=true,
	linkcolor=UMBlack,
	citecolor=UMBlack,
	urlcolor=UMc16,
	filecolor=UMc7,
	pdfborder={0 0 0},
}

\addtolength{\cftsubsubsectionindent}{2em}
\renewcommand\cftsubsubsectionfont{\footnotesize}

\settocdepth{subsubsection} % Table of Contents depth also upto subsections

%% TODO: Figure out a way to either use titletoc or etoc
%% to have more control an a better table of contens

%% Modify the Table of Contents slightly
\renewcommand*{\cftchaptername}{Chapter\space}
\renewcommand*{\cftappendixname}{Appendix\space}
\renewcommand*{\cftsectionname}{§\space}
\renewcommand*{\cftsubsubsectionname}{-\space}


% \newcommand{\removelinebreaks}[1]{%
% 	\begingroup\def\\{ }#1\endgroup}
%
% % Controls space between one reference and the next
% \setlength{\bibsep}{6.5pt}
%
% \addto{\captionsenglish}{\renewcommand{\bibname}{References}} % because we are using babel we need this
%                                                              % instead of just redefining bibname
% \setlength{\bibitemsep}{\onelineskip}


\setmainfont{lato} % Setup the main font as Lato
% \setmonofont{menlo} % Setup the mono font as Iosevka

\setdefaultlanguage{english} % Set the default language of the document

%% These are the most common types of statements
\newtheorem{conj}{Conjecture}
\newtheorem{cor}{Corollary}
\newtheorem{defn}{Definition}
\newtheorem{lma}{Lemma}
\newtheorem{prop}{Proposition}
\newtheorem{thm}{Theorem}

%% **** Document Options (End) ****

%% **** Environments (Start) ****

%% *** Title Page (Start) ***
%% The front page
\renewcommand{\maketitle}{
\begingroup
% \vspace*{1in}
\textcolor{UMRed}{\rule{\linewidth}{2pt}}

\vspace*{2.5pt}\hfill\textcolor{UMRed}{\rule{2pt}{0.5\textheight}}
\hspace*{2ex}
\parbox[b][0.5\textheight][t]{0.8\textwidth}{
	\setlength{\parindent}{0pt}
    	\sffamily
	{\huge\bfseries{\begin{Spacing}{1.15}\textcolor{UMRed}{\@title}\end{Spacing}}}
	\vspace*{1ex}
    	{\Large\textit{\@tagline}}\\[0.5in]
    	{\LARGE\bfseries{\@author}} \\[1.5ex]
    	{\large\bfseries{\@authorid}} \\[1.5ex]
    	\@ifundefined{@department}{}{{\large \@department} \\[1ex]}
    	\@ifundefined{@faculty}{}{{\large \@faculty} \\[1ex]}
    	\vfill
	{\bfseries \@courseworkdate}\\[\baselineskip]
	\parbox[t]{0.80\linewidth}{\footnotesize\textit{A \@doctype\ submitted in fulfilment of study unit \@subjectcode}.}
}
\thispagestyle{empty}
% \if@openright\cleardoublepage\else\clearpage\fi
\endgroup}
%% *** Title Page (End) ***

%% **************** Environments (End) *******************

\endinput % Stop reading the class file
