[
    CHAT_CLIENT.BF
    Connects to Port 8000 (80 * 100).
    Logic: Connect -> [ Receive Loop -> Send Loop ]
]

(STEP 1: SET PORT 8000)
++++++++                (Set Cell 0 to 8)
[>++++++++++<-]         (Cell 1 = 80)
>                       (Move to Cell 1)

(STEP 2: CONNECT TO SERVER)
&                       (New Command: Connect to localhost:8000)

(STEP 3: CHAT LOOP)
(We use Cell 2 for I/O and Cell 3 for Temp storage)
>+                      (Set Cell 2 to 1 to enter Main Loop)
[
    [-]                 (Clear Cell 2)
    
    ( --- RECEIVE PHASE --- )
    (Logic: Read char from Net. If Newline(10), break to Send Phase. Else print.)
    
    +                   (Set Cell 2 to 1 to enter Recv Loop)
    [
        [-]             (Clear Cell 2)
        % , %           (Read from Network)
        
        (Check for 0/Disconnect)
        [
            (Copy Cell 2 to Cell 3 to check for Newline)
            [>+>+<<-]>>[<<+>>-] (Destructive copy C2->C3, restore C2)
            
            (Subtract 10 from Cell 3)
            ----------
            
            [           (If Cell 3 is NOT zero, i.e., not a newline)
                < .     (Print Cell 2 to Screen)
                >[-]    (Clear Cell 3)
            ]
            
            <           (Back to Cell 2)
            
            (Check if Cell 3 was zero? Hard in BF without more temp cells.)
            (SIMPLIFIED LOGIC: We just subtract 10 from the input itself.)
            (If result is 0, we exit loop. If not, we add 10 back and print.)
            
            ----------  (Subtract 10)
            [           (If not Newline)
                ++++++++++ (Add 10 back)
                .       (Print to Screen)
                [-]     (Clear Cell to Repeat Loop)
                % , %   (Read next char)
            ]
            (If we are here, the loop exited because Cell 2 was 0 or became 0)
            (Wait, the inner loop structure is getting complex. Let's simplify.)
        ]
        
        (If we read a 0/Disconnect, we should probably stop the program)
        (But for this chat, we assume the loop broke because of Newline logic above)
    ]
    
    (Print a local newline for formatting)
    ++++++++++ . [-]
    
    ( --- SEND PHASE --- )
    (Logic: Read from Kbd. Send. If Newline, Send Newline and Break to Recv.)
    
    +                   (Enter Send Loop)
    [
        [-]             (Clear)
        ,               (Read from Keyboard)
        
        (Check for Newline 10)
        ----------      (Subtract 10)
        [               (If not Newline)
            ++++++++++  (Add 10 back)
            % . %       (Send to Network)
            [-]         (Clear)
            ,           (Read next)
        ]
        
        (If Newline was typed)
        ++++++++++      (Restore the 10)
        % . %           (Send the Newline to Server)
        [-]             (Clear to exit Send Loop)
    ]
    
    (Loop back to Receive Phase)
    +
]