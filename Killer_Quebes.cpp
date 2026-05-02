#include <TL-Engine.h> // TL-Engine include file and namespace
using namespace tle;

// Constants for various values used in the game
const float marbleXvalue = 0.0f;       // Initial X position of the marble
const float marbleYValue = 2.0f;
const float marbleZvalue = 0.0f;
const float marbleRadius = 2.0f;     // Radius of the marble for collision detection

const float arrowXvalue = 0.0f;    // Initial X position of the arrow
const float arrowYValue = 2.0f;
const float arrowZValue = -10.0f;
const float arrowMaxRot = 45.0f;        // Maximum rotation for the arrow in degrees

const float movementSpeed = 0.3f;     // Speed of movement for marble and arrow

const float blockXValue = -42.0f;    // Initial X position for blocks
const float blockYValue = 0.0f;
const float blockZValue = 120.0f;     // Z position for the first row of blocks
const float blockGap = 12.0f;        // Gap between blocks in a row
const float blockRadius = 6.0f;     // Radius used for collision detection with blocks
const float block2ZValue = 132.0f; // Z position for the second row of blocks

const float barrierXLeftValue = -60.0f; // X position of left barriers
const float barrierXRightValue = 60.0f; // X position of right barriers
const float barrierYValue = 0.0f;
const float barrierSpacing = 18.0f; // Spacing between barriers


const float floorXValue = 0.0f;     // X position for the floor
const float floorYvalue = -10.0f;
const float floorZvalue = 0.0f;

const float skyXvalue = 0.0f;     // X position for the skybox
const float skyYvalue = -1000.0f;
const float skyZvalue = 0.0f;

const float boundry = 140.0f;     // Boundary limit for the marble's movement

const float camXValue = 0.0f;
const float camYValue = 30.0f;
const float camZValue = -75.0f;
const float camRotateXValue = 10.0f;

const int totalBlocks = 8; // Total blocks in each row

const float row1XZValue = 0.0f;
const float row2XZValue = 0.0f;
const float row12YValue = -999.0f;

const int totalBarriers = 10;   // Total number of barriers

int main()
{
    // Create the 3D engine and start in windowed mode
    I3DEngine* myengine = New3DEngine(kTLX);
    myengine->StartWindowed();

    // Add media folder for loading assets
    myengine->AddMediaFolder(".\\Media");

    // Define game states
    enum GameState
    {
        StateReady,   // Ready to fire the marble
        StateFiring,  // Marble is moving
        StateContact, // Marble has collided with something
        StateOver     // Game over state
    };

    // Define block states
    enum BlockState {
        BlockUntouched, // Block has not been hit
        BlockHitOnce,   // Block has been hit once
        BlockDeleted    // Block is removed after being hit twice
    };

    // Structure to store details about blocks
    struct BlockDetails {
        IModel* blockModel;    // Pointer to the block model
        bool isHit;            // Flag to check if the block has been hit
        BlockState blockState; // Current state of the block
    };

    // Initialize the game state
    GameState currentState = StateReady;

    // Load meshes for models
    IMesh* Blockmesh = myengine->LoadMesh("Block.x");
    IMesh* Barriermesh = myengine->LoadMesh("Barrier.x");
    IMesh* Marblemesh = myengine->LoadMesh("Marble.x");
    IMesh* Arrowmesh = myengine->LoadMesh("Arrow.x");
    IMesh* Floormesh = myengine->LoadMesh("Floor.x");
    IMesh* Skyboxmesh = myengine->LoadMesh("Skybox_Hell.x");
    IMesh* Dummymesh = myengine->LoadMesh("Dummy.x");

    // Create models for game objects
    IModel* marbleModel = Marblemesh->CreateModel(marbleXvalue, marbleYValue, marbleZvalue);
    IModel* arrowModel = Arrowmesh->CreateModel(arrowXvalue, arrowYValue, arrowZValue);
    IModel* skyboxModel = Skyboxmesh->CreateModel(skyXvalue, skyYvalue, skyZvalue);
    IModel* floorModel = Floormesh->CreateModel(floorXValue, floorYvalue, floorZvalue);

    // Add a dummy model for the arrow to follow
    IModel* dummyModel = Dummymesh->CreateModel(marbleXvalue, marbleYValue, marbleZvalue);
    arrowModel->AttachToParent(dummyModel);

    // Initialize blocks for two rows
    BlockDetails row1Blocks[totalBlocks];
    BlockDetails row2Blocks[totalBlocks];

    float initialX = blockXValue;
    float hitBlocksCount = 0; // Track the number of blocks hit

    for (int i = 0; i < totalBlocks; ++i)
    {
        // Create blocks for row 1
        row1Blocks[i].blockModel = Blockmesh->CreateModel(initialX + i * blockGap, blockYValue, blockZValue);
        row1Blocks[i].isHit = false;
        row1Blocks[i].blockState = BlockUntouched;

        // Create blocks for row 2
        row2Blocks[i].blockModel = Blockmesh->CreateModel(initialX + i * blockGap, blockYValue, block2ZValue);
        row2Blocks[i].isHit = false;
        row2Blocks[i].blockState = BlockUntouched;
    }

    // Create barriers on the left and right
    IModel* leftBarriers[totalBarriers];
    IModel* rightBarriers[totalBarriers];

    for (int i = 0; i < totalBarriers; ++i)
    {
        float barrierZPos = i * barrierSpacing;
        leftBarriers[i] = Barriermesh->CreateModel(barrierXLeftValue, barrierYValue, barrierZPos);
        rightBarriers[i] = Barriermesh->CreateModel(barrierXRightValue, barrierYValue, barrierZPos);

        // Apply a different skin for the last few barriers
        if (i >= totalBarriers - 4)
        {
            leftBarriers[i]->SetSkin("barrier1a.BMP");
            rightBarriers[i]->SetSkin("barrier1a.BMP");
        }
    }

    // Set up the camera
    ICamera* mycamera = myengine->CreateCamera(kManual);
    mycamera->SetPosition(camXValue, camYValue,camZValue);
    mycamera->RotateX(camRotateXValue);

    // Variable to track the dummy's rotation
    float dummyRotation = 0.0f;

    // Load font for displaying text
    IFont* text = myengine->LoadFont("Arial", 36);

    // Main game loop
    while (myengine->IsRunning())
    {
        myengine->DrawScene();

        // Exit the game if Escape key is pressed
        if (myengine->KeyHeld(Key_Escape))
        {
            break;
        }

        // Handle different game states
        switch (currentState)
        {
        case StateReady:
            // Logic for aiming and preparing to fire
            arrowModel->AttachToParent(dummyModel);

            // Rotate the dummy model to aim the arrow
            if (myengine->KeyHeld(Key_Z) && dummyRotation < arrowMaxRot)
            {
                dummyRotation += movementSpeed;
            }
            if (myengine->KeyHeld(Key_X) && dummyRotation > -arrowMaxRot)
            {
                dummyRotation -= movementSpeed;
            }

            dummyModel->ResetOrientation();
            marbleModel->ResetOrientation();
            dummyModel->RotateY(dummyRotation);
            marbleModel->RotateY(dummyRotation);

            // Fire the marble when Space is pressed
            if (myengine->KeyHit(Key_Space))
            {
               // arrowModel->DetachFromParent();
                currentState = StateFiring;
            }
            break;

        case StateFiring:
            // Move the marble forward
            marbleModel->MoveLocalZ(movementSpeed);

            // Check for collisions with blocks and barriers
            for (int i = 0; i < totalBlocks; ++i)
            {
                // Check collision with row 1 blocks
                if (row1Blocks[i].blockState != BlockDeleted)
                {
                    float dx = row1Blocks[i].blockModel->GetX() - marbleModel->GetX();
                    float dz = row1Blocks[i].blockModel->GetZ() - marbleModel->GetZ(); 
                    float distance = sqrt(dx * dx + dz * dz);

                    if (distance <= blockRadius + marbleRadius)
                    {
                        if (row1Blocks[i].blockState == BlockUntouched)
                        {
                            row1Blocks[i].blockModel->SetSkin("tiles_red.jpg");
                            row1Blocks[i].blockState = BlockHitOnce;
                            hitBlocksCount++;
                            currentState = StateContact;
                            break;
                        }
                        else if (row1Blocks[i].blockState == BlockHitOnce)
                        {
                            row1Blocks[i].blockModel->SetPosition(row1XZValue, row12YValue, row1XZValue);
                            row1Blocks[i].blockState = BlockDeleted;
                            hitBlocksCount++;
                            currentState = StateContact;
                            break;
                        }
                    }
                }

                // Check collision with row 2 blocks
                if (row2Blocks[i].blockState != BlockDeleted)
                {
                    float dx2 = row2Blocks[i].blockModel->GetX() - marbleModel->GetX();
                    float dz2 = row2Blocks[i].blockModel->GetZ() - marbleModel->GetZ();
                    float distance2 = sqrt(dx2 * dx2 + dz2 * dz2);

                    if (distance2 <= blockRadius + marbleRadius)
                    {
                        if (row2Blocks[i].blockState == BlockUntouched)
                        {
                            row2Blocks[i].blockModel->SetSkin("tiles_red.jpg");
                            row2Blocks[i].blockState = BlockHitOnce;
                            hitBlocksCount++;
                            currentState = StateContact;
                            break;
                        }
                        else if (row2Blocks[i].blockState == BlockHitOnce)
                        {
                            row2Blocks[i].blockModel->SetPosition(row2XZValue, row12YValue, row2XZValue);
                            row2Blocks[i].blockState = BlockDeleted;
                            hitBlocksCount++;
                            currentState = StateContact;
                            break;
                        }
                    }
                }

                // Check collisions with barriers
                for (int i = 0; i < totalBarriers; ++i)
                {
                    float dxLeft = leftBarriers[i]->GetX() - marbleModel->GetX();
                    float dzLeft = leftBarriers[i]->GetZ() - marbleModel->GetZ();
                    float distanceLeft = sqrt(dxLeft * dxLeft + dzLeft * dzLeft);

                    float dxRight = rightBarriers[i]->GetX() - marbleModel->GetX();
                    float dzRight = rightBarriers[i]->GetZ() - marbleModel->GetZ();
                    float distanceRight = sqrt(dxRight * dxRight + dzRight * dzRight);

                    if (distanceLeft <= blockRadius || distanceRight <= blockRadius)
                    {
                        currentState = StateContact;
                        break;
                    }
                }
            }

            // Check if the marble goes out of bounds
            if (marbleModel->GetLocalZ() >= boundry)
                currentState = StateContact;

            break;

        case StateContact:
            // Reset the marble to its starting position
            if (marbleModel->GetLocalZ() > 0.0f)
            {
                marbleModel->MoveLocalZ(-movementSpeed);
            }
            else
            {
                currentState = StateReady;
            }

            break;

        case StateOver:
            // Display the game-over screen
            marbleModel->SetPosition(marbleXvalue, marbleYValue, marbleZvalue);
            marbleModel->SetSkin("glass_green.jpg");
            text->Draw("Game Over", 500, 300, kWhite);

            break;
        }

        // Check if all blocks have been deleted
        bool allBlocksDeleted = true;

        for (int i = 0; i < totalBlocks; ++i)
        {
            if (row1Blocks[i].blockState != BlockDeleted || row2Blocks[i].blockState != BlockDeleted)
            {
                allBlocksDeleted = false;
                break;
            }
        }

        // If all blocks are deleted, transition to the game-over state
        if (allBlocksDeleted)
        {
            currentState = StateOver;
        }
    }

    // Clean up the engine and exit
    myengine->Delete();
}