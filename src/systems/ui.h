#ifndef DIDDLEDOODLEDUEL_UI_H
#define DIDDLEDOODLEDUEL_UI_H

struct UISystem {
    void render(engine::IDrawHandler& drawHandler, const std::string& title) {
        DrawFPS(20, 100);
        drawHandler.drawText(title, Vector2 {20.0F, 20.0F }, 24, BLACK);
        drawHandler.drawText("Use A or D to update the rotation", Vector2 { 20.0F , 50.0F}, 18, BLACK);
    }
};

#endif // DIDDLEDOODLEDUEL_UI_H
