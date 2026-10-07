#include "SelectWorldScreen.h"

#include <stdint.h>
#include <time.h>

#include <algorithm>
#include <chrono>
#include <format>

#include "app/common/Network/GameNetworkManager.h"
#include "app/common/UI/All Platforms/UIStructs.h"
#include "app/common/UI/ConsoleUIController.h"
#include "minecraft/IGameServices.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/gui/Button.h"
#include "minecraft/client/gui/ConfirmScreen.h"
#include "minecraft/client/gui/CreateWorldScreen.h"
#include "minecraft/client/gui/MessageScreen.h"
#include "minecraft/client/gui/RenameWorldScreen.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/client/gui/ScrolledSelectionList.h"
#include "minecraft/locale/Language.h"
#include "minecraft/network/INetworkService.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/util/Log.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/storage/LevelStorageSource.h"
#include "minecraft/world/level/storage/LevelSummary.h"
#include "platform/storage/storage.h"
#include "util/StringHelpers.h"

extern std::string g_pendingWorldName;

SelectWorldScreen::SelectWorldScreen(Screen* lastScreen) {
    // 4J - added initialisers
    title = "Select world";
    done = false;
    selectedWorld = -1;
    worldSelectionList = nullptr;
    isDeleting = false;
    deleteButton = nullptr;
    selectButton = nullptr;
    renameButton = nullptr;

    this->lastScreen = lastScreen;
}

void SelectWorldScreen::init() {
    Log::info("SelectWorldScreen::init() START\n");
    Language* language = Language::getInstance();
    title = language->getElement("selectWorld.title");

    worldLang = language->getElement("selectWorld.world");
    conversionLang = language->getElement("selectWorld.conversion");
    loadLevelList();

    worldSelectionList = new WorldSelectionList(this);
    worldSelectionList->init(&buttons, BUTTON_UP_ID, BUTTON_DOWN_ID);

    postInit();
}

void SelectWorldScreen::loadLevelList() {
    LevelStorageSource* levelSource = minecraft->getLevelSource();
    levelList = levelSource->getLevelList();
    //	Collections.sort(levelList);	// 4J - TODO - get sort functor etc.
    selectedWorld = -1;
}

std::string SelectWorldScreen::getWorldId(int id) {
    if (id < 0 || id >= (int)levelList->size()) return "";
    return levelList->at(id)->getLevelId();
}

std::string SelectWorldScreen::getWorldName(int id) {
    if (id < 0 || id >= (int)levelList->size()) return "";
    std::string levelName = levelList->at(id)->getLevelName();

    if (levelName.length() == 0) {
        levelName = worldLang + " " + std::to_string(id + 1);
    }

    return levelName;
}

void SelectWorldScreen::postInit() {
    Language* language = Language::getInstance();

    buttons.push_back(selectButton = new Button(
        BUTTON_SELECT_ID, width / 2 - 154, height - 52, 150,
        20, language->getElement("selectWorld.select")));
    buttons.push_back(renameButton = new Button(
        BUTTON_RENAME_ID, width / 2 - 154, height - 28, 70,
        20, language->getElement("selectWorld.rename")));
    buttons.push_back(deleteButton = new Button(
        BUTTON_DELETE_ID, width / 2 - 74, height - 28, 70, 20,
        language->getElement("selectWorld.delete")));
    buttons.push_back(new Button(BUTTON_CREATE_ID, width / 2 + 4, height - 52,
                                 150, 20,
                                 language->getElement("selectWorld.create")));
    buttons.push_back(new Button(BUTTON_CANCEL_ID, width / 2 + 4, height - 28,
                                 150, 20, language->getElement("gui.cancel")));

    selectButton->active = false;
    deleteButton->active = false;
    renameButton->active = false;
}

void SelectWorldScreen::buttonClicked(Button* button) {
    if (!button->active) return;

    if (button->id == BUTTON_DELETE_ID) {
        std::string worldName = getWorldName(selectedWorld);
        if (worldName != "") {
            isDeleting = true;
            Language* language = Language::getInstance();
            std::string q = language->getElement("selectWorld.deleteQuestion");
            std::string w = "'" + worldName + "' " +
            language->getElement("selectWorld.deleteWarning");
            std::string y = language->getElement("selectWorld.deleteButton");
            std::string n = language->getElement("gui.cancel");

            ConfirmScreen* confirmScreen =
            new ConfirmScreen(this, q, w, y, n, selectedWorld);
            minecraft->setScreen(confirmScreen);
        }
    } else if (button->id == BUTTON_SELECT_ID) {
        worldSelected(selectedWorld);
    } else if (button->id == BUTTON_CREATE_ID) {
        minecraft->setScreen(new CreateWorldScreen(this));
    } else if (button->id == BUTTON_RENAME_ID) {
        minecraft->setScreen(
            new RenameWorldScreen(this, getWorldId(selectedWorld)));
    } else if (button->id == BUTTON_CANCEL_ID) {
        minecraft->setScreen(lastScreen);
    } else {
        worldSelectionList->buttonClicked(button);
    }
}

void SelectWorldScreen::mouseClicked(int x, int y, int buttonNum) {
    if (worldSelectionList != nullptr &&
        worldSelectionList->mouseClicked(x, y, buttonNum)) {
        return;
        }
        Screen::mouseClicked(x, y, buttonNum);
}

void SelectWorldScreen::worldSelected(int id) {
    if (done) return;
    if (id < 0 || id >= (int)levelList->size()) return;

    done = true;
    minecraft->gameMode = nullptr;

    LevelSummary* summary = levelList->at(id);
    std::string worldFolderName = summary->getLevelId();
    std::string worldName = summary->getLevelName();
    if (worldName.empty()) worldName = worldFolderName;

    PlatformStorage.ResetSaveData();
    PlatformStorage.SetSaveTitle((char*)worldName.c_str());
    g_pendingWorldName = worldFolderName;

    NetworkGameInitData* param = new NetworkGameInitData();
    param->seed = 0;
    param->findSeed = true;
    param->saveData = nullptr;
    param->texturePackId = 0;

    GameType* gameMode = summary->getGameMode();
    int gameModeId = (gameMode != nullptr) ? gameMode->getId() : 0;

    gameServices().setGameHostOption(eGameHostOption_Difficulty,
                                     minecraft->options->difficulty);
    gameServices().setGameHostOption(eGameHostOption_GameType, gameModeId);
    gameServices().setGameHostOption(eGameHostOption_CheatsEnabled,
                                     summary->hasCheats());

    param->settings = gameServices().getGameHostOption(eGameHostOption_All);
    param->xzSize = LEVEL_MAX_WIDTH;
    param->hellScale = HELL_LEVEL_MAX_SCALE;

    NetworkService.HostGame(0, false, false, MINECRAFT_NET_MAX_PLAYERS, 0);
    NetworkService.FakeLocalPlayerJoined();

    LoadingInputParams* loadingParams = new LoadingInputParams();
    loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->lpParam = param;

    UIFullscreenProgressCompletionData* completionData =
    new UIFullscreenProgressCompletionData();
    completionData->bShowBackground = true;
    completionData->bShowLogo = true;
    completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
    completionData->iPad = 0;  // Set valid player index
    loadingParams->completionData = completionData;

    ui.NavigateToScene(0, eUIScene_FullscreenProgress, loadingParams);

    Language* language = Language::getInstance();
    minecraft->setScreen(
        new MessageScreen(language->getElement("menu.loadingLevel")));
}

void SelectWorldScreen::confirmResult(bool result, int id) {
    if (isDeleting) {
        isDeleting = false;
        if (result) {
            LevelStorageSource* levelSource = minecraft->getLevelSource();
            levelSource->deleteLevel(getWorldId(id));
            loadLevelList();
        }
        minecraft->setScreen(this);
    }
}

void SelectWorldScreen::render(int xm, int ym, float a) {
    // fill(0, 0, width, height, 0x40000000);
    renderDirtBackground(0);
    if (worldSelectionList != nullptr) {
        worldSelectionList->render(xm, ym, a);
    }

    drawCenteredString(font, title, width / 2, 20, 0xffffff);
    Screen::render(xm, ym, a);

    // 4J - debug code - remove
    if (0) {
        static int count = 0;
        static bool forceCreateLevel = false;
        if (count++ >= 100) {
            if (!forceCreateLevel && levelList->size() > 0) {
                // 4J Stu - For some obscures reason the "delete" button is
                // called "renameButton" and vice versa. if( levelList->size() >
                // 2 && deleteButton->active )
                //{
                //	this->selectedWorld = 2;
                //	count = 0;
                //	buttonClicked(deleteButton);
                //}
                // else
                if (levelList->size() > 1 && renameButton->active) {
                    this->selectedWorld = 1;
                    count = 0;
                    buttonClicked(renameButton);
                } else if (selectButton->active == true) {
                    this->selectedWorld = 0;
                    buttonClicked(selectButton);
                    // this->worldSelected( 0 );
                } else {
                    selectButton->active = true;
                    deleteButton->active = true;
                    renameButton->active = true;
                    count = 0;
                }
            } else {
                Log::info(
                    "SelectWorldScreen::render minecraft->setScreen(new "
                    "CreateWorldScreen(this))\n");
                minecraft->setScreen(new CreateWorldScreen(this));
            }
        }
    }
}

SelectWorldScreen::WorldSelectionList::WorldSelectionList(
    SelectWorldScreen* sws)
    : ScrolledSelectionList(sws->minecraft, sws->width, sws->height, 32,
                            sws->height - 64, 36) {
    parent = sws;
}

int SelectWorldScreen::WorldSelectionList::getNumberOfItems() {
    return (int)this->parent->levelList->size();
}

void SelectWorldScreen::WorldSelectionList::selectItem(int item,
                                                       bool doubleClick) {
    parent->selectedWorld = item;
    bool active = (item >= 0 && item < getNumberOfItems());

    if (parent->selectButton) parent->selectButton->active = active;
    if (parent->deleteButton) parent->deleteButton->active = active;
    if (parent->renameButton) parent->renameButton->active = active;

    if (doubleClick && active) {
        parent->worldSelected(item);
    }
}

bool SelectWorldScreen::WorldSelectionList::isSelectedItem(int item) {
    return item == parent->selectedWorld;
}

int SelectWorldScreen::WorldSelectionList::getMaxPosition() {
    return (int)parent->levelList->size() * 36;
}

void SelectWorldScreen::WorldSelectionList::renderBackground() {
    parent->renderBackground();  // 4J - was
                                 // SelectWorldScreen.this.renderBackground();
}

void SelectWorldScreen::WorldSelectionList::renderItem(int i, int x, int y,
                                                       int h, Tesselator* t) {
    LevelSummary* levelSummary = parent->levelList->at(i);

    std::string name = levelSummary->getLevelName();
    if (name.length() == 0) {
        name = parent->worldLang + " " + std::to_string(i + 1);
    }

    std::string metadata = levelSummary->getLevelId();
    int64_t size = levelSummary->getSizeOnDisk();
    std::string sizeStr =
    std::format("{:.1f} MB", (float)size / (1024.0f * 1024.0f));

    metadata += " (" + sizeStr + ")";

    parent->drawString(parent->font, name, x + 2, y + 1, 0xffffff);
    parent->drawString(parent->font, metadata, x + 2, y + 12, 0x808080);

    if (levelSummary->isRequiresConversion()) {
        parent->drawString(parent->font, parent->conversionLang, x + 2, y + 22,
                           0x808080);
    }
}
