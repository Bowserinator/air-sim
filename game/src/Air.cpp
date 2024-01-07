#include "Air.h"

#include <memory>

Air::Air() {
    clear();
}

void Air::clear() {
    memset(cells, 0.0f, sizeof(cells));
    memset(out_cells, 0.0f, sizeof(cells));
}

void Air::update() {

}
