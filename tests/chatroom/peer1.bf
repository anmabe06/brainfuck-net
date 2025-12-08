[-]                 Clear Cell 0
>++++++++           Cell 1 = 8
[<++++++++++>-]     Cell 0 = 80 (8 * 10)
<                   Move back to Cell 0 (Value 80)
^                   Start Server on Port 8000
%                   Switch to Network Mode

>                   Move to Cell 1
+                   Set Cell 1 to 1 to enter the loop initially

[                   MAIN LOOP
    [-]             Clear Cell 1 (Flag/Temp)
    
    >               Move to Cell 2 (will be used as marker)
    [-]             Clear Cell 2 (marker = 0)
    >               Move to Cell 3 (Input Buffer Start)
    [-]             Clear Cell 3 (start with clean buffer)
    
    ,               RECEIVE LOOP Read from Socket
    ----------      Check Newline
    [               If Not Newline
       ++++++++++   Restore
       >            Next Cell
       [-]          Clear Next Cell (Safety)
       ,            Read Next
       ----------   Check Newline
    ]
    
                    We hit Newline (Current Cell is 0)
    
    ++++++++++      Restore Newline 10
    > [-]           Set next cell to 0 (Terminator)
    <               Move back to Newline
    
    %               SWITCH TO CONSOLE MODE
    
    < [ < ]         REWIND Moves left until it hits a 0
                    Since Cell 2 is 0 it stops at Cell 2
    
    [-]             Clear Cell 2
    
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.    Print LEFT BRACKET
    ------.                                                                                         Print U
    ++++++++++++++++++++++++++++++.                                                                 Print s
    --------------.                                                                                 Print e
    +++++++++++++.                                                                                  Print r
    ---------------------.                                                                          Print RIGHT BRACKET
    -----------------------------------.                                                            Print COLON
    --------------------------.                                                                     Print SPACE
    [-]             Clear Cell 2 back to 0 (Critical for navigation)
    
    >               Move to Buffer (Cell 3)
    
    [               PRINT LOOP
       .            Print Char
       >            Next
    ]
                    Loop stops at 0 terminator
    < [ < ]         Rewind to Cell 2 (Which is 0)
    
    ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ .    Print RIGHT ARROW
    >
    [-]
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
       ----------   Check Newline
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
       .            Send Char to Peer2
       >            Next Char
    ]
                    Loop stops at the 0 terminator
    
    < [ < ]         Rewind to Cell 2 (Which is 0)
    <               Move to Cell 1
    <               Move to Cell 0 (Sentinel 80)
    >+              Ready for next message (Pointer ends at Cell 1)
]

