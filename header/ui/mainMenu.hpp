#pragma once
#ifndef MAIN_MENU_HPP
#define MAIN_MENU_HPP

class MainMenu {
    public:
        void Update();
        void Draw();
        bool ShouldStartGame() const;
        bool ShouldOpenCollections() const;
        bool ShouldOpenHangar() const;
        bool ShouldOpenSettings() const;
        bool ShouldOpenRunHistory() const;
        bool collections = false;
        bool hangar = false;
        bool runHistory = false;
        bool settings = false;
        bool startGame = false;
    private:
        int menuSelection = 0;
        int menuOptions = 5;
        bool gameStarted = false;
        bool showControls = false;
    };


#endif // MAIN_MENU_HPP