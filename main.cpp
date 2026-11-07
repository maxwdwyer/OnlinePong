#include "headers.h"

using namespace std;

enum NetworkMode
{
    NETWORK_NONE,
    NETWORK_HOST,
    NETWORK_CLIENT
};

NetworkMode networkMode = NETWORK_NONE;

int timeMine = 0;
int fps = 300;
float roundPace = 25;
int room = 2;
//0 = local game 
//1 = local score menu
//2 = main menu
//3 = settings
//4 = online settings room
//5 = online game
bool gameStart = false;

Vector2 score;

Sound paddleHit;
Sound roomSwitch;
Sound scorePointSound;
Sound thud;

float LengthDirX(double len, double dir);
float LengthDirY(double len, double dir);
Vector2 LengthDirXY(double len, double dir);
float clamp(float value, float minimum, float maximum);

class paddle;
class ball;

void scorePoint(ball& Ball);

struct OnlineData {
    int value;
    Vector2 ballPos;
};

// COLORS ------------------------------------------------------------------
Color paddleColor = { 86, 156, 214, 255 };
Color bgColor = { 159, 227, 213, 255 };
Color ballColor = { 86, 156, 214, 255 };
Color bgDetailColor = { 78, 201, 176, 255 };
Color particleColor1 = { 218, 109, 208, 255 };
Color particleColor2 = { 203, 94, 193, 255 };
Color particleColor3 = { 233, 124, 223, 255 };
Color particleColor4 = { 255, 175, 247, 255 };
//{156, 220, 254, 255}

// CLASSES ------------------------------------------------------------------
class particle {
public:
    Color myColor = particleColor1;
    Vector2 position = { 0, 0 };
    Vector2 scale = { 25, 25 };
    float power = .05;
    Rectangle rectangle = { position.x, position.y, scale.x, scale.y };

    float direction = 0;
    float speed = 0;

    void step() {
        Vector2 move = LengthDirXY(speed, direction + 90.0f);
        position.x += move.x;
        position.y += move.y;
        scale.x -= power;
        scale.y -= power * 2;
        rectangle = { position.x, position.y, scale.x, scale.y };
        DrawRectanglePro(rectangle, { 0, 0 }, direction, myColor);
    };
};

vector<particle> particleList;

class paddle {
private:
    Vector2 scale = { 12, 124 };
public:
    Vector2 position = { 0, 0 };
    Rectangle rectangle = {
        position.x - (scale.x / 2), position.y - (scale.y / 2), // CORNER ONE
        scale.x, scale.y // WIDTH AND HEIGHT
    };

    void step() {
        float xScale = clamp(124 - (0.25f * (1 + roundPace)), 48, 124);
        scale = { 12, xScale };

        position.y = clamp(position.y, (scale.y / 2) + 8, 592 - (scale.y / 2));
        rectangle = {
            position.x - (scale.x / 2), position.y - (scale.y / 2), // CORNER ONE
            scale.x, scale.y // WIDTH AND HEIGHT
        };

        DrawRectangleRec(rectangle, paddleColor);
    };
};

class ball {
public:
    Vector2 position = { 400, 303 };
    float radius = clamp(3250.0 / (roundPace + 300.0), 4, 16);
    float speed = .5 + (roundPace / 150);
    float direction = 0;
    void newParticle(paddle Paddle) {
        particle newParticle;
        float ranx = GetRandomValue(-7, 7);
        float rany = GetRandomValue(-24 - (speed / 1.75), 24 + (speed / 1.75));
        float ranSizeX = GetRandomValue(4, 9) + (speed / 3.5);
        float ranSizeY = GetRandomValue(10, 30) + speed;
        float partSpeed = GetRandomValue(1, 7) / 15.0f + (speed / 27.5);
        float ranColor = GetRandomValue(0, 3);
        if (ranColor == 0) newParticle.myColor = particleColor1;
        if (ranColor == 1) newParticle.myColor = particleColor2;
        if (ranColor == 2) newParticle.myColor = particleColor3;
        if (ranColor == 3 && speed > 2.5) newParticle.myColor = particleColor4;
        newParticle.position = { position.x + ranx, position.y + rany };
        newParticle.direction = atan2(Paddle.position.y - newParticle.position.y, Paddle.position.x - newParticle.position.x) * (180 / PI) + 90;
        newParticle.scale = { ranSizeX, ranSizeY };
        newParticle.speed = partSpeed;
        particleList.push_back(newParticle);
    };
    void step(paddle paddleOne, paddle paddleTwo) {
        radius = clamp(3250.0 / (roundPace + 300.0), 4, 16);
        DrawCircleV(position, radius, ballColor);

        if (gameStart == true) {
            Vector2 move = LengthDirXY(speed, direction);
            Vector2 testPosition = Vector2Add(position, move);

            if (CheckCollisionCircleRec(testPosition, radius, paddleOne.rectangle) == true) {
                direction = (position.y - paddleOne.position.y) / (paddleOne.rectangle.height / 2.0f) * 75.0f;
                roundPace += 13 * (speed / 1.1);
                for (int i = 0; i < clamp(floor(speed * 1.25), 0, 7) + 2; i++) {
                    newParticle(paddleOne);
                };
                SetSoundVolume(paddleHit, ((480 - clamp(480 - roundPace, 0, 480)) + 150) / 825);
                PlaySound(paddleHit);
            }
            else if (CheckCollisionCircleRec(testPosition, radius, paddleTwo.rectangle) == true) {
                direction = 180.0f - ((position.y - paddleTwo.position.y) / (paddleTwo.rectangle.height / 2.0f) * 75.0f);
                roundPace += 13 * (speed / 1.1);
                for (int i = 0; i < clamp(floor(speed * 1.25), 0, 7) + 2; i++) {
                    newParticle(paddleTwo);
                };
                SetSoundVolume(paddleHit, ((480 - clamp(500 - roundPace, 0, 480)) + 150) / 825);
                PlaySound(paddleHit);
            }
            else if (testPosition.x < radius || testPosition.x > 800 - radius) {
                scorePoint(*this);
                return;
            }
            else if (testPosition.y < radius || testPosition.y > 600 - radius)
            {
                direction = -direction;
                PlaySound(thud);
            }
            else position = testPosition;

            speed = clamp(.5 + (roundPace / 150), .5, 15);
        }
    };
    void reset() {
        position.x = 400;
        position.y = 303;
        roundPace = 25;
        speed = .5 + (roundPace / 150);
    };
};

class button {
public:
    Rectangle rec = { 0, 0, 100, 100 };
    string text = "";
    virtual void step() {
        DrawRectangle(rec.x, rec.y, rec.width, rec.height, bgDetailColor);
        DrawText(TextFormat(" %s ", text.c_str()), rec.x - 24, rec.y + 4, 90, ballColor);
    };
    bool checkClicked() {
        float x = GetMouseX();
        float y = GetMouseY();
        if (x > rec.x && x < rec.x + rec.width && y > rec.y && y < rec.y + rec.height) return true;
        else return false;
    };
};

class inputBox : public button {
public:
    bool typing = false;
    void step() override {
        if (typing == false) DrawRectangle(rec.x, rec.y, rec.width, rec.height, bgDetailColor);
        float time = sin(GetTime() * 2) * 10;
        if (typing == true) DrawRectangle((int)(rec.x - (time / 2)), (int)(rec.y - (time / 2)), (int)(rec.width + time), (int)(rec.height + time), bgDetailColor);
        DrawText(TextFormat(" %s ", text.c_str()), rec.x - 24, rec.y + 4, 90, ballColor);
        if (checkClicked() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (typing == true) {
                typing = false;
            }
            else {
                typing = true;
            };
        };
        if (typing == true) {
            int key = GetCharPressed();

            if (IsKeyPressed(KEY_BACKSPACE) && !text.empty()) text.pop_back();
            if (key > 0)
            {
                text += (char)key;
                key = GetCharPressed();
            };
        };
    };
};

int main() {
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    ENetAddress address;
    ENetHost* server = nullptr;
    ENetHost* client = nullptr;
    ENetPeer* clientPeer = nullptr;

    bool clientConnected = false;

    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bobby.");
    SetTargetFPS(fps);
    InitAudioDevice();

    Font font = GetFontDefault();

    paddleHit = LoadSound("hit.mp3");
    roomSwitch = LoadSound("menu.mp3");
    SetSoundVolume(roomSwitch, .5);
    scorePointSound = LoadSound("score.mp3");
    SetSoundVolume(scorePointSound, .7);
    thud = LoadSound("thud.mp3");
    SetSoundVolume(thud, .45);
    SetSoundPitch(thud, .45);

    paddle paddleOne;
    paddle paddleTwo;
    ball Ball;

    paddleOne.position = Vector2{ 50, 300 };
    paddleTwo.position = Vector2{ 750, 300 };

#pragma region BUTTONS
    button playLocal;
    playLocal.rec = { 24, 230, 573, 100 };
    playLocal.text = "Play Locally";

    button playEnet;
    playEnet.rec = { 24, 355, 514, 100 };
    playEnet.text = "Play online";

    button openSettings;
    openSettings.rec = { 24, 480, 406, 100 };
    openSettings.text = "Settings";

    button createHost;
    createHost.rec = { 24, 100, 575, 100 };
    createHost.text = "create host";

    button joinServer;
    joinServer.rec = { 24, 230, 573, 100 };
    joinServer.text = "join server";

    inputBox IPbox;
    IPbox.rec = { 24, 355, 514, 100 };
    IPbox.text = "192.168.1.41";

#pragma endregion

    while (WindowShouldClose() == false) {
        float speed = clamp(.5 * ((75 + roundPace) / 75), .5, 3);

        timeMine++;
        if (timeMine == 301) timeMine = 0;

        ENetEvent event;

        //HOST CODE --------------------------------------------------------------------
        if (networkMode == NETWORK_HOST && server != nullptr) {
            int32_t value = 0;
            Vector2 ballPos = Ball.position;

            if (room == 5 && gameStart == true) {
                if (IsKeyDown('W') || IsKeyDown(KEY_UP)) value = -1;
                if (IsKeyDown('S') || IsKeyDown(KEY_DOWN)) value = 1;
            };

            if (IsKeyDown(KEY_SPACE) && room == 5) {
                gameStart = true;
                value = 2;
            }

            if (clientPeer != nullptr)
            {
                OnlineData data = { 0 };
                data.value = value;
                data.ballPos = ballPos;

                ENetPacket* packet = enet_packet_create(
                    &data,
                    sizeof(OnlineData),
                    ENET_PACKET_FLAG_RELIABLE);

                enet_peer_send(clientPeer, 0, packet);
                enet_host_flush(server);
            }

            if (room == 5 && gameStart == true) {
                if (IsKeyDown('W') || IsKeyDown(KEY_UP)) paddleOne.position.y -= speed * .95;
                if (IsKeyDown('S') || IsKeyDown(KEY_DOWN)) paddleOne.position.y += speed * .95;
            };

            while (enet_host_service(server, &event, 0) > 0)
            {
                switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("A new client connected from %x:%u.\n",
                        event.peer->address.host,
                        event.peer->address.port);
                    //event.peer -> data = "Client information";
                    clientPeer = event.peer;
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    int32_t valueRecieved;
                    memcpy(&valueRecieved, event.packet->data, sizeof(valueRecieved));
                    if (valueRecieved == 1) paddleTwo.position.y += speed * .95;
                    if (valueRecieved == -1) paddleTwo.position.y -= speed * .95;

                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("%s disconnected.\n", event.peer->data);

                    event.peer->data = NULL;
                    break;
                }
            }
        };

        // CLIENT --------------------------------------------------------------------
        if (networkMode == NETWORK_CLIENT && client != nullptr) {

            ENetEvent event;
            while (enet_host_service(client, &event, 0) > 0)
            {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_CONNECT:
                    cout << "Connected to server!\n";
                    clientConnected = true;
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    OnlineData valueRecieved;
                    memcpy(&valueRecieved, event.packet->data, sizeof(valueRecieved));
                    if (valueRecieved.value == 2) gameStart = true;
                    if (valueRecieved.value == 1) paddleOne.position.y += speed * .95;
                    if (valueRecieved.value == -1) paddleOne.position.y -= speed * .95;
                    Ball.position = valueRecieved.ballPos;
                    
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    cout << "Disconnected\n";
                    break;
                }
            }

            int32_t value = 0;
            if (room == 5 && gameStart == true) {
                if (IsKeyDown('W') || IsKeyDown(KEY_UP))
                {
                    value = -1;
                    paddleTwo.position.y -= speed * .95;
                }
                if (IsKeyDown('S') || IsKeyDown(KEY_DOWN))
                {
                    value = 1;
                    paddleTwo.position.y += speed * .95;
                }
            };

            if (clientConnected == true && clientPeer != nullptr) {
                ENetPacket* packet = enet_packet_create(
                    &value,
                    sizeof(int),
                    ENET_PACKET_FLAG_RELIABLE);

                enet_peer_send(clientPeer, 0, packet);
                enet_host_flush(client);
            }
        }

        if (room == 0) {
            if (IsKeyDown(KEY_SPACE)) {
                gameStart = true;
            };

            if (IsKeyDown('W')) paddleOne.position.y -= speed * .95;
            if (IsKeyDown(KEY_UP)) paddleTwo.position.y -= speed * .95;
            if (IsKeyDown('S')) paddleOne.position.y += speed * .95;
            if (IsKeyDown(KEY_DOWN)) paddleTwo.position.y += speed * .95;
        }

        //SWITCHING ROOMS -------------------------------------------
        if (IsKeyDown(KEY_SPACE) && room == 1) {
            PlaySound(roomSwitch);
            room = 0;
            Ball.position = { 400, 303 };
            roundPace = 0;
            paddleOne.position.y = 300;
            paddleTwo.position.y = 300;
        };
        if (IsKeyPressed(KEY_BACKSPACE) && room == 0 && IPbox.typing == false)
        {
            PlaySound(roomSwitch);
            room = 1;
        }
        else if (IsKeyPressed(KEY_BACKSPACE) && room != 0 && room != 2 && IPbox.typing == false)
        {
            PlaySound(roomSwitch);
            room = 2;
            if (server != nullptr) {
                enet_host_destroy(server);
                server = nullptr;
                networkMode = NETWORK_NONE;
            };
            if (client != nullptr) {
                enet_host_destroy(client);
                client = nullptr;
                networkMode = NETWORK_NONE;
            };
    };

        //BUTTON PRESSED CHECK --------------------------------------
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            switch (room) {
            case 2:
                if (playLocal.checkClicked() == true)
                {
                    room = 1;
                    PlaySound(roomSwitch);
                }
                if (playEnet.checkClicked() == true)
                {
                    room = 4;
                    PlaySound(roomSwitch);
                }
                if (openSettings.checkClicked() == true)
                {
                    room = 3;
                    PlaySound(roomSwitch);
                }
                break;
            case 4:
                //CREATE SERVER
                if (createHost.checkClicked() == true) {
                    if (server != nullptr) {
                        enet_host_destroy(server);
                        server = nullptr;
                    };

                    address.host = ENET_HOST_ANY;
                    address.port = 1235;

                    server = enet_host_create(&address, 1, 2, 0, 0);
                    if (server == NULL) {
                        fprintf(stderr,
                            "An error occurred while trying to create an ENet server host.\n");
                        exit(EXIT_FAILURE);
                    };
                    networkMode = NETWORK_HOST;
                    room = 5;
                };

                //JOIN SERVER
                if (joinServer.checkClicked() == true)
                {
                    client = enet_host_create(NULL, 1, 2, 0, 0);
                    if (client == NULL) {
                        fprintf(stderr,
                            "An error occurred while trying to create an ENet client host.\n");
                        exit(EXIT_FAILURE);
                    };

                    enet_address_set_host(&address, IPbox.text.c_str());
                    address.port = 1235;

                    clientPeer = enet_host_connect(client, &address, 2, 0);
                    if (clientPeer == nullptr)
                    {
                        fprintf(stderr, "No available peers for initiating an ENet connection.\n");
                        exit(EXIT_FAILURE);
                    }
                    room = 5;
                    networkMode = NETWORK_CLIENT;
                };
                break;
            default:
                break;

                //if (testInput.checkClicked() == true) testInput.typing = true;
                //else testInput.typing = false;
            }
        };

        BeginDrawing();
        ClearBackground(bgColor);

        if (room != 4) IPbox.typing = false;

        switch (room) {
        case 0:
            DrawRectangleLinesEx({ 0, 0, 800, 600 }, 6, bgDetailColor);
            DrawText("PONG", 198, 234, 150, bgDetailColor);
            DrawRectangle(397, 0, 6, 600, bgDetailColor);
            DrawRectangleLinesEx({ -100, 50, 200, 500 }, 6, bgDetailColor);
            DrawRectangleLinesEx({ 700, 50, 200, 500 }, 6, bgDetailColor);
            for (int i = 0; i < particleList.size(); i++) {
                particleList.at(i).step();
                if (particleList.at(i).scale.x < .5 || particleList.at(i).scale.y < .5) {
                    particleList.erase(particleList.begin() + i);
                    i--;
                }
            };
            paddleOne.step();
            paddleTwo.step();
            Ball.step(paddleOne, paddleTwo);
            DrawFPS(10, 10);
            break;
        case 1:
            DrawText("PLAYER\nONE", 16, 16, 100, bgDetailColor);
            DrawText("     TWO\nPLAYER", 372, 392, 100, bgDetailColor);
            DrawText(TextFormat("%.0f", score.x), 16, 370, 250, ballColor);
            DrawText(TextFormat("%.0f", score.y), 504, 0, 250, ballColor);
            DrawLineEx({ 325, 610 }, { 475, -10 }, 24, bgDetailColor);
            DrawText("VS", 300, 225, 150, bgDetailColor);
            break;
        case 2:
            Ball.direction = 0;
            score = { 0, 0 };
            roundPace = 25;
            DrawTextPro(font, "PONG", { 400, 125 }, { 250, 100 }, sin(GetTime() / 1.25) * 1.5, 200, 10, ballColor);
            playLocal.step();
            playEnet.step();
            openSettings.step();
            break;
        case 4:
            createHost.step();
            joinServer.step();
            IPbox.step();
            break;
        case 5:
            DrawRectangleLinesEx({ 0, 0, 800, 600 }, 6, bgDetailColor);
            DrawText("PONG", 198, 234, 150, bgDetailColor);
            DrawRectangle(397, 0, 6, 600, bgDetailColor);
            DrawRectangleLinesEx({ -100, 50, 200, 500 }, 6, bgDetailColor);
            DrawRectangleLinesEx({ 700, 50, 200, 500 }, 6, bgDetailColor);
            for (int i = 0; i < particleList.size(); i++) {
                particleList.at(i).step();
                if (particleList.at(i).scale.x < .5 || particleList.at(i).scale.y < .5) {
                    particleList.erase(particleList.begin() + i);
                    i--;
                }
            };
            paddleOne.step();
            paddleTwo.step();
            Ball.step(paddleOne, paddleTwo);
            DrawFPS(10, 10);
            break;
        default:
            break;
        }
        EndDrawing();
    }

    if (server != nullptr) {
        enet_host_destroy(server);
    }

    UnloadSound(paddleHit);
    CloseAudioDevice();
    CloseWindow();
    atexit(enet_deinitialize);
};

float clamp(float value, float minimum, float maximum) {
    if (value < minimum) value = minimum;
    if (value > maximum) value = maximum;
    return value;
};

float LengthDirX(double len, double dir) {
    return len * cos(dir * PI / 180);
};

float LengthDirY(double len, double dir) {
    return len * sin(dir * PI / 180);
};

Vector2 LengthDirXY(double len, double dir) {
    float x = len * cos(dir * PI / 180);
    float y = len * sin(dir * PI / 180);
    return Vector2{ x, y };
};

void scorePoint(ball& Ball) {
    if (Ball.position.x < 400) {
        score.y += 1;
        Ball.direction = 0;
        PlaySound(scorePointSound);
    }
    else if (Ball.position.x > 400) {
        score.x += 1;
        Ball.direction = 180;
        PlaySound(scorePointSound);
    }
    Ball.reset();
    room = 1;
};