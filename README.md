# C
This repository contains four standalone C programs that demonstrate numerical methods and parallel computing using POSIX threads. Each task folder includes source code, test files and a brief report.

1. **Linear Regression**  
   • Read coordinate data from text files, compute slope (b) and intercept (a) via least-squares formula, output the line equation and predict y for user-supplied x.

2. **Pi Calculation (Leibniz Series)**  
   • Approximate π using the Leibniz infinite series, accept iteration count and thread count as command-line arguments, dynamically slice work across threads.

3. **Polynomial Derivatives**  
   • Parse functions and x-values from text files, compute symbolic derivatives, evaluate at each x, write derivative expressions and results to output files, parallelise via threads.

4. **Box Blur Filter**  
   • Decode a PNG into a pixel array, apply a 3×3 box-blur filter to each pixel (handling edges), leverage multithreading for workload distribution, and write the blurred image back to disk.

**Requirements**  
• C11 compiler, POSIX threads, libpng  
