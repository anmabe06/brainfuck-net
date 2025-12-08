[-]                 Clear Cell 0
>++++++++           Cell 1 = 8
[<++++++++++>-]     Cell 0 = 80 (8 * 10)
<                   Move back to Cell 0 (Value 80)
^                   Start Server on Port 8000
%                   Switch to Network Mode

>                   Move to Cell 1
+                   Set Cell 1 to 1 to enter the loop initially
[                   MAIN LOOP
    ,               Read byte from socket (Waits for data)
    .               Echo byte back to socket
]                   If read failed (connection closed), cell is 0, loop exits

