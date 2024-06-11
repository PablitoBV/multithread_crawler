\documentclass{article}
\usepackage{hyperref}
\usepackage{listings}
\usepackage{xcolor}

\title{Web Crawler Project}
\author{[Your Name]}
\date{\today}

\begin{document}

\maketitle

\section{Overview}

This project is a multi-threaded web crawler that processes web pages to extract links and save performance metrics. It uses \texttt{libcurl} for downloading web content, \texttt{gumbo-parser} for parsing HTML, and custom data structures for concurrent link processing.

\section{Prerequisites}

\begin{itemize}
    \item \texttt{libcurl}
    \item \texttt{gumbo-parser}
    \item A C++11 compatible compiler
    \item \texttt{make}
\end{itemize}

\section{Project Structure}

\begin{itemize}
    \item \textbf{main.cpp}: Main source file for the web crawler.
    \item \textbf{ConcurrentQueue.h}: Header file for thread-safe queue implementation.
    \item \textbf{StripedHashSet.h}: Header file for thread-safe hash set implementation.
    \item \textbf{Makefile}: Build configuration for the project.
\end{itemize}

\section{Execution}

\subsection{Building the Project}

To compile the project, use the following commands:

\begin{lstlisting}[language=bash]
make clean
make
\end{lstlisting}

\subsection{Running the Web Crawler}

To run the web crawler, execute the compiled binary with the required arguments:

\begin{lstlisting}[language=bash]
./web <NUMBER_OF_THREADS> <MAX_LINKS_TO_PROCESS> <OUTPUT_CSV_FILE>
\end{lstlisting}

For example, to run the crawler with 4 threads and process up to 100 links, saving results to \texttt{crawler\_results.csv}:

\begin{lstlisting}[language=bash]
./web 4 100 crawler_results.csv
\end{lstlisting}

\section{Notes}

\begin{itemize}
    \item Ensure that \texttt{libcurl} and \texttt{gumbo-parser} are installed and properly linked during compilation.
    \item Adjust \texttt{maxLinksToProcess} in \texttt{main.cpp} if different from the command line argument.
    \item The initial URL is set to \url{https://en.wikipedia.org/wiki/Main_Page}. Modify it if necessary.
\end{itemize}

\section{License}

This project is licensed under the MIT License.

\section{Author}

[Your Name] - [Your Contact Information]

\end{document}
