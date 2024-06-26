
# skippityC

skippityC is a complete implementation of the board game "Skippity" in C language with AI addition.




## Features
- **Two-player mode:** Compete against your friends!
- **Single-player mode:** Play against an AI opponent with selectable difficulty levels (Medium, Extreme).
- **AI vs. AI mode:** Watch two AIs compete against each other.
- **Variable board size:** Choose a board size from 6x6 to 20x20 for a more challenging experience.
- **Randomized board setup:** The pieces on the board is randomized for every game.
- **Visual game board:** The game uses colored characters to represent each piece type for better experience!
- **Undo/Redo functionality:** Players can undo and redo moves during their turn. Everyone can make mistakes right?
- **Save/Load game:** Save progress and resume games later.
- **Game over control:** The game automatically ends when there are no more possible moves.
- **User-friendly interface:** A simple and easy-to-use console interface.
- **Two different control mode:** You can enjoy a fast paced game with wasd keys or more attentive moves with coordinates!
- **Artificial Intelligence:** The AI opponent is designed with a sophisticated algorithm to make it challenging to defeat!
- **Tutorial:** Learn the rules and gameplay mechanics with a comprehensive in-game tutorial.

  
## Rules

The rules of the Skippity game are very simple:

### Game Board

It is played on a square game board of size 6-20. There are 5 pieces (Pieces can also be called 'Skippers') named A B C D E.

Each square on the game board is randomly filled with one of the 5 different pieces.

The middle 2x2 part is left empty at the beginning of the game to be able to make a move.

### Movement

When it's your turn, you jump a piece on the board over another piece to an empty square, and you add the piece you jumped over to your score.

Jumps can only be made vertically and horizontally. You can only jump over an adjacent piece.

If you are not satisfied with your move, you can undo it with the 'u' key, and if you change your mind, you can redo the previous move with the 'r' key.

You have 1 undo and 1 redo right per turn.

If you can make another move from the square you jumped from, you can continue jumping. If you can't, you need to pass the turn to the other player.

### Score 

If you collect one of each of the A B C D E pieces, this is called a \"set\". Each collected set is worth 100 points.

Each extra piece you have outside of the sets is worth 1 point, regardless of the type of piece.

### Determining the winner

The game ends automatically when there are no more moves to be made at the end of the game.

The player with the most points wins.

### Gameplay mechanics and keys

At the beginning of the game, you need to choose the game mode. Press '1' for two-player game, '2' to play against the computer.

You can choose between playing with wasd keys or coordinates in the next menu.

Then you will be asked to enter the size of the game board. The game board must be entered between 6-20. Otherwise, the game will not start.

You can press 's' to save the game before or after making a move, and 'l' to load any saved game.

An example board is such as:

![board](https://raw.githubusercontent.com/mertgulerx/skippityC/main/screenshots/336614726-97f3a871-b40b-4a3b-9b3e-f17772e8ceef.png)


After this stage, you need to enter coordinates to make a move.

        Player 1's turn
        Which piece to move (row / column):
        Where to place the piece (row / column):

Please enter coordinates in this format: 'x y', '1 3' . Other entries are invalid.

### Jumping tutorial

Examine the following example rows and columns:

             1   2   3   4   5   6  
           +---+---+---+---+---+---+
         1 | A | B |   |   |   |   |
           +---+---+---+---+---+---+

        1 1 -----> 1 3

             1   2   3   4   5   6  
           +---+---+---+---+---+---+
         1 |   |   | A |   |   |   |
           +---+---+---+---+---+---+
        
        Move successful!
        
        A:0 B:1 C:0 D:0 E:0
        Score: 1
        Set: 0
        Extra Skipper: 1
        
        
             1   2   3   4   5   6  
           +---+---+---+---+---+---+
         1 | A |   |   |   |   |   |
           +---+---+---+---+---+---+
         2 | C |   |   |   |   |   |
           +---+---+---+---+---+---+
         3 |   |   |   |   |   |   |
           +---+---+---+---+---+---+
        
        1 1 -----> 3 1
        
             1   2   3   4   5   6  
           +---+---+---+---+---+---+
         1 |   |   |   |   |   |   |
           +---+---+---+---+---+---+
         2 |   |   |   |   |   |   |
           +---+---+---+---+---+---+
         3 | A |   |   |   |   |   |
           +---+---+---+---+---+---+
        
        Move successful!
        
        A:0 B:0 C:1 D:0 E:0
        Score: 1
        Set: 0
        Extra Skipper: 1

Now you are ready! Don't forget to play a game against the computer at extreme difficulty to test the intelligence of my AI!


![Gif](https://raw.githubusercontent.com/mertgulerx/skippityC/main/screenshots/ezgif-1-e508be8d76.gif)


##
Bu oyun Yıldız Teknik Üniversitesi, Yapısal Programlama dersi projesidir.