#include "globals.h"
#include <QGuiApplication>
#include <QScreen>

namespace Globals {
    namespace {
        bool s_debug = false;
        float s_scale = 2.0f;
        float s_speed = 1.0f;
        int s_encounter_level_low = 5;
        int s_encounter_level_high = 5;
        bool s_exp_share = false;
    }

    bool debug() { return s_debug; }
    void debug(bool value) { s_debug = value; }

    float scale() { return s_scale; }
    void scale(float value) { s_scale = value; }

    float animationSpeed() { return s_speed; }
    void animationSpeed(float value) { s_speed = value; }

    int encounterLvlLow(){ return s_encounter_level_low;}
    void encounterLvlLow(int value) { s_encounter_level_low = value;}

    int encounterLvlHigh() { return s_encounter_level_high;}
    void encounterLvlHigh(int value) { s_encounter_level_high = value;}

    bool expShare() { return s_exp_share;}
    void expShare(bool value) { s_exp_share = value; }

    const QRect& screenGeometry() {
        static const QRect geometry = QGuiApplication::primaryScreen()->geometry();
        return geometry;
    }
}
