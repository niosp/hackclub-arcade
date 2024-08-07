## Debugging Progress
Problem: The example ROM (just an IBM logo) looks somewhat scrambled on the screen, so there must be a parsing error somewhere
![IBM logo looks scrambled](image-1.png)

Added checks to disable out-of-bounds write (screenshot line 347)

![Drawing Processor](image.png)


The next problem:

![Bigger window, rectangle size of 1*1px is much larger being displayed (lol?)](image-2.png)

### Issue is now resolved

![IBM Logo](image-3.png)