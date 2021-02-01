# CONCLUSION AND SOME HINTS

1. Do not rush to write the advanced version directly, start from the implicit list to explicit list, and then to the segregated list.(the segregated list one is actually simpler than the explicit list)
   
2. write the heapchecker, it will be really useful. I do not believe this one originally, but it turns out to help me find one bug that I took 2 days to debug. (In explicit list version, I put some part of updating predecessor and successor procedure in the `mm_free` function,  which results in that the newly extended memory cannot be added to the free list.)
   
3. Some casting is necessray. Originally I do not have the `SETP` (set pointer macro), and the update of the predecessor and successor is not quite right.

4. wrap all memory operation in the macro and wrap the operation and operator with parentheses carefully.