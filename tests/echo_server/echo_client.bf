[-]                 Clear Cell 0
>++++++++           Cell 1 is 8
[<++++++++++>-]     Cell 0 is 80 Pointer ends at Cell 1 which is 0

<                   Move back to Cell 0 which is 80
&                   Connect to Port 8000


>+ 
[                   MAIN LOOP (Infinite)
    [-]             Clear Cell 1 (Flag/Temp)

    ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ .    Print RIGHT ARROW
    >
    ++++++++++++++++++++++++++++++++ .                                  Print SPACE 
    [-]             Clear Prompt Cell (Cell 2 is now 0)
    
    >               Move to Cell 3 (Input Buffer Start)
    
    ,               READ LOOP Read char from Keyboard
    ----------      Check Newline
    [               If Not Newline
       ++++++++++   Restore
       >            Next Cell
       [-]          Clear Next Cell (Safety)
       ,            Read Next
        ----------  Check Newline
    ]
    
                    We hit Newline (Current Cell is 0)
    
    ++++++++++      Restore Newline 10
    > [-]           Set next cell to 0 (Terminator)
    <               Move back to Newline
   
 
    < [ < ]         REWIND Moves left until it hits a 0
                    Since Cell 2 is 0 it stops at Cell 2
    
    %               SWITCH TO NETWORK MODE
    
    >               Move 1 right to Cell 3 (Buffer Start)
    
    [               SEND LOOP
       .            Send Char to Server
       >            Next Char
    ]
                    Loop stops at the 0 terminator

Now we RECEIVE the echo    
    < [ < ]         Rewind to Cell 2 (0)
    >               Move 1 right to Buffer Start to overwrite
    
    ,               RECV LOOP Read from Socket
    ----------      Check Newline
    [
    
   ++++++++++   Restore
       >            Next
       [-]          Clear Next Cell (Safety)
       ,            Read Next
       ----------   Check Newline
    ]
    
                
    Got Newline from Server (Current Cell is 0)
    
    ++++++++++      Restore Newline 10
    > [-]           Set next cell to 0 (Terminator)
    <               Back to Newline
    
    %               SWITCH TO CONSOLE MODE
    
 
   < [ < ]          Rewind to Cell 2 (0)
    
                    Print "Server: "
    
    [-]             Clear
    
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.  Print S
    ++++++++++++++++++.                                                                   Print e
    +++++++++++++.                                                                        Print r
    ++++.                                                                                 Print v
    -----------------.                                                                    Print e
    +++++++++++++.                                                                        Print r
    --------------------------------------------------------.                             Print COLON
    --------------------------.                                                           Print SPACE
    [-]             Clear Cell 2 back to 0 (Critical for navigation)
    
    >               Move to Buffer (Cell 3)
    
    [               PRINT LOOP
       .            Print Char
       >            Next
    ]
                    Loop stops at 0 terminator.
    < [ < ]         Rewind to Cell 2 (Which is 0)
    <               Move to Cell 1
    <               Move to Cell 0 (Sentinel 80)
    >+              Ready for next message (Pointer ends at Cell 1)
]