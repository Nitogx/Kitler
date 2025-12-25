![Logo](logo/Group1.png)
---
***Made for game devs, built for apps***

## Language Overview
Kitler is an interpreted language designed for game development and UI applications with minimal boilerplate.

---

## Core Syntax Rules

### Comments
```kt
<-- Single line comment -->
<-- 
   Multi-line comment
   spans multiple lines
-->
```

### Including System Libraries
```kt
including System.Interface#
including Windows.NET8#
including System.IO#
including System.Audio#
including System.Physics#
```
**Note:** `#` at the end marks priority includes - interpreter checks these first.

---

## Project Structure

### Project Space Definition
```kt
projectSpace MyGameProject [
    <-- Project-level configuration -->
    
    MyGameProject.WhenRan[
        StartAll.Components()
        App.New = New WindowComponent("My Game", false, false, Windowed, 1280x720)
    ]
    
    <-- Functions and logic go here -->
]
```

### ProjectName.ktconfig Format
```json
{
    "projectName": "MyGame",
    "dotnetVersion": "8",
    "projectType": "game",
    "autoOptimized": true,
    "includes": [
        "System.Interface",
        "Windows.NET8",
        "System.Audio",
        "System.Physics"
    ],
    "entryPoint": "Template.kt",
    "outputType": "gui"
}
```

---

## Data Types & Variables

### Variable Declaration
```kt
NewVar playerHealth = 100
NewVar playerName = "Hero"
NewVar isAlive = true
NewVar position = Vec2(0, 0)
NewVar items = List["Sword", "Shield", "Potion"]
NewVar playerData = Map{"health": 100, "mana": 50}
```

### Type System
- **Numbers:** `int`, `float`, `double`
- **Text:** `string`
- **Boolean:** `true`, `false`
- **Collections:** `List[...]`, `Map{...}`
- **Vectors:** `Vec2(x, y)`, `Vec3(x, y, z)`
- **Colors:** `Color.Red`, `RGB(255, 0, 0)`, `RGBA(255, 0, 0, 128)`

---

## Functions

### Function Definition
```kt
NewFunc CalculateDamage(attackPower, defense) (
    NewVar damage = attackPower - defense
    return Max(damage, 0)
)

NewFunc Update() (
    player.x += velocity.x
    player.y += velocity.y
    CheckCollisions()
)

<-- No-return function -->
NewFunc PrintMessage(msg) (
    Console.Write(msg)
)
```

---

## Control Flow

### Conditionals
```kt
if playerHealth <= 0 run:
    GameOver()
    return
end

if score > highScore run:
    highScore = score
    SaveHighScore()
else:
    Console.Write("Try again!")
end

<-- Multiple conditions -->
if playerX > 100 and playerY < 50 run:
    TriggerEvent()
end
```

### Loops
```kt
<-- For loop -->
for i in Range(0, 10) run:
    Console.Write(i)
end

<-- While loop -->
while gameRunning run:
    Update()
    Draw()
end

<-- Foreach loop -->
foreach item in inventory run:
    Console.Write(item.name)
end
```

### Switch Statements
```kt
switch gameState run:
    case "menu":
        DrawMenu()
        break
    case "playing":
        UpdateGame()
        break
    case "paused":
        DrawPauseScreen()
        break
    default:
        ErrorHandler()
end
```

---

## GUI Components

### Window Creation
```kt
App.New = New WindowComponent("Title", false, false, Borderless, Maximized)
<-- WindowComponent(title, isMessageBox, hasYesNo, type, size) -->
<-- Types: Windowed, Borderless, Fullscreen, Maximized, Minimized -->
<-- Sizes: 1920x1080, 1280x720, 800x600, or Custom(w, h) -->
```

### Drawing Primitives
```kt
NewFunc CreateGui() (
    StartComponent(Windows.NET, Version=8)
    
    when Windows.NET8 run:
        <-- Rectangle: [width, height, color, draggable] -->
        DrawRectangle[100, 100, Color.Blue, false]
        DrawRectangle[200, 150, RGB(255, 128, 0), true]
        
        <-- Circle: [radius, color, filled] -->
        DrawCircle[50, Color.Green, true]
        
        <-- Line: [x1, y1, x2, y2, color, thickness] -->
        DrawLine[0, 0, 100, 100, Color.White, 2]
        
        <-- Text: [text, x, y, fontSize, color, font] -->
        DrawText["Score: 100", 10, 10, 24, Color.White, "Arial"]
        
        <-- Image: [path, x, y, width, height] -->
        DrawImage["assets/player.png", 100, 100, 64, 64]
    end
)
```

### UI Components
```kt
<-- Button: ButtonComponent(text, clickable, onClick, visible) -->
Windows.NET8.AddButton = New ButtonComponent("Start Game", true, StartGame(), true)
Windows.NET8.AddButton = New ButtonComponent("Quit", true, App.Exit(), true)

<-- Text Input: InputComponent(placeholder, x, y, width, onEnter) -->
Windows.NET8.AddInput = New InputComponent("Enter name...", 100, 100, 200, SaveName())

<-- Slider: SliderComponent(min, max, default, x, y, onChange) -->
Windows.NET8.AddSlider = New SliderComponent(0, 100, 50, 100, 200, UpdateVolume())

<-- Panel: PanelComponent(x, y, width, height, color, children) -->
Windows.NET8.AddPanel = New PanelComponent(0, 0, 300, 500, Color.DarkGray, [
    New ButtonComponent("Option 1", true, Option1(), true),
    New ButtonComponent("Option 2", true, Option2(), true)
])

<-- Label: LabelComponent(text, x, y, fontSize, color) -->
Windows.NET8.AddLabel = New LabelComponent("Health: 100", 10, 50, 16, Color.Red)
```

---

## Game-Specific Features

### Sprite System
```kt
NewVar player = New Sprite("assets/player.png", 100, 100, 64, 64)
player.SetAnimation("walk", ["walk1.png", "walk2.png", "walk3.png"], 10)
player.PlayAnimation("walk")
player.SetVelocity(5, 0)
player.SetCollider(Rectangle, 64, 64)

NewFunc Update() (
    player.Move()
    player.UpdateAnimation()
    
    if Input.IsKeyDown(Keys.Space) run:
        player.Jump(10)
    end
)
```

### Input Handling
```kt
NewFunc HandleInput() (
    if Input.IsKeyDown(Keys.W) run:
        player.y -= 5
    end
    
    if Input.IsKeyPressed(Keys.Space) run:
        player.Jump()
    end
    
    if Input.IsMouseButtonDown(MouseButton.Left) run:
        NewVar mousePos = Input.GetMousePosition()
        SpawnParticle(mousePos.x, mousePos.y)
    end
    
    if Input.IsGamepadButtonPressed(Gamepad.A) run:
        player.Attack()
    end
)
```

### Physics & Collision
```kt
including System.Physics#

NewVar player = New PhysicsBody(100, 100, 64, 64, Mass=1.0)
player.SetGravity(9.8)
player.SetFriction(0.5)
player.ApplyForce(Vec2(10, 0))

NewFunc CheckCollisions() (
    NewVar collisions = Physics.CheckCollisions(player)
    
    foreach obj in collisions run:
        if obj.tag == "enemy" run:
            TakeDamage(10)
        end
    end
)
```

### Audio System
```kt
including System.Audio#

NewVar bgMusic = Audio.Load("music/theme.mp3")
NewVar jumpSound = Audio.Load("sfx/jump.wav")

Audio.Play(bgMusic, Loop=true, Volume=0.7)
Audio.PlayOneShot(jumpSound, Volume=1.0)
Audio.Stop(bgMusic)
```

---

## Special Keywords & Built-ins

### System Functions
```kt
Console.Write("Hello")          <-- Print to console -->
Console.Read()                  <-- Read user input -->
App.Exit()                      <-- Close application -->
Time.DeltaTime()               <-- Frame delta time -->
Time.GetTime()                 <-- Current time in seconds -->
Random.Range(0, 100)           <-- Random number -->
Math.Sqrt(16)                  <-- Math operations -->
File.Read("data.txt")          <-- File I/O -->
File.Write("data.txt", content)
```

### Lifecycle Hooks
```kt
projectSpace MyGame [
    MyGame.WhenRan[
        StartAll.Components()
        App.New = New WindowComponent("Game", false, false, Windowed, 1280x720)
        Initialize()
    ]
    
    <-- Called every frame -->
    MyGame.Update[
        HandleInput()
        UpdatePhysics()
        CheckCollisions()
    ]
    
    <-- Called every frame after Update -->
    MyGame.Draw[
        ClearScreen(Color.Black)
        DrawSprites()
        DrawUI()
    ]
    
    <-- Called when app closes -->
    MyGame.OnExit[
        SaveGameData()
        Cleanup()
    ]
]
```

---

## Advanced Features

### Events & Callbacks
```kt
NewEvent OnPlayerDeath
NewEvent OnScoreChanged(newScore)

NewFunc SubscribeToEvents() (
    OnPlayerDeath.Subscribe(GameOver)
    OnScoreChanged.Subscribe(UpdateScoreUI)
)

NewFunc TakeDamage(amount) (
    playerHealth -= amount
    if playerHealth <= 0 run:
        OnPlayerDeath.Invoke()
    end
)
```

### Classes & Objects
```kt
NewClass Enemy [
    NewVar health = 100
    NewVar damage = 10
    NewVar position = Vec2(0, 0)
    
    NewFunc TakeDamage(amount) (
        this.health -= amount
        if this.health <= 0 run:
            this.Die()
        end
    )
    
    NewFunc Die() (
        SpawnDeathEffect(this.position)
        Destroy(this)
    )
]

NewVar goblin = New Enemy()
goblin.health = 50
goblin.TakeDamage(25)
```

### Async Operations
```kt
NewAsync LoadAssets() (
    NewVar texture = await File.LoadAsync("player.png")
    NewVar audio = await Audio.LoadAsync("music.mp3")
    return true
)

NewFunc Initialize() (
    NewVar success = await LoadAssets()
    if success run:
        StartGame()
    end
)
```

---

## CLI Commands

### Project Management
```bash
# Create new project
kt new MyGame

# Generate config (interactive)
kt --config

# Generate config (auto-optimized)
kt --config=auto

# Run project
kt run --file=MyGame.kt

# Run in GUI interpreter
kt gui --file=MyGame.kt

# Build standalone executable
kt build --project=MyGame --output=MyGame.exe

# Clean build files
kt clean
```

### Config Options
When running `kt --config`, the interpreter asks:
1. .NET version (6, 7, 8, 9)
2. Project type (game, terminal, gui, both)
3. Editor preference (gui interpreter, external editor)
4. Auto-optimize (yes/no)

---

## Example: Complete Game

```kt
including System.Interface#
including Windows.NET8#
including System.Audio#
including System.Physics#

projectSpace AppleGame [
    NewVar apple
    NewVar score = 0
    NewVar gameRunning = true
    
    AppleGame.WhenRan[
        StartAll.Components()
        App.New = New WindowComponent("Apple Catcher", false, false, Windowed, 800x600)
        Initialize()
    ]
    
    NewFunc Initialize() (
        apple = New Sprite("apple.png", 400, 0, 64, 64)
        apple.SetVelocity(0, 2)
        SpawnApple()
    )
    
    NewFunc SpawnApple() (
        apple.x = Random.Range(0, 800)
        apple.y = 0
    )
    
    AppleGame.Update[
        if gameRunning run:
            apple.Move()
            
            if Input.IsMouseButtonPressed(MouseButton.Left) run:
                NewVar mousePos = Input.GetMousePosition()
                if CheckClick(mousePos, apple) run:
                    score += 1
                    SpawnApple()
                end
            end
            
            if apple.y > 600 run:
                gameRunning = false
                ShowGameOver()
            end
        end
    ]
    
    AppleGame.Draw[
        ClearScreen(Color.SkyBlue)
        apple.Draw()
        DrawText["Score: " + score, 10, 10, 32, Color.Black, "Arial"]
    ]
    
    NewFunc CheckClick(mousePos, sprite) (
        if mousePos.x >= sprite.x and mousePos.x <= sprite.x + sprite.width run:
            if mousePos.y >= sprite.y and mousePos.y <= sprite.y + sprite.height run:
                return true
            end
        end
        return false
    )
    
    NewFunc ShowGameOver() (
        Windows.NET8.AddButton = New ButtonComponent("Play Again", true, RestartGame(), true)
    )
    
    NewFunc RestartGame() (
        score = 0
        gameRunning = true
        Initialize()
    )
]
```

---

## Compiler & Interpreter Architecture

The KT interpreter is written in C and compiled with GCC for cross-platform support:
- **Lexer** - Tokenizes `.kt` source
- **Parser** - Builds AST from tokens
- **Interpreter** - Executes AST nodes
- **Runtime** - Provides built-in functions
- **Bridge** - Interfaces with .NET for GUI/system calls

Priority includes (`#`) are parsed first and cached for faster execution.
