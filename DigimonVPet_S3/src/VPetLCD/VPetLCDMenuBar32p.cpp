#include "VPetLCDMenuBar32p.h"


        /**
         * Constructor - Creates a Menubar with _numberOfMenuItems menuItems. The bar
         * Shows _iconsPerPage icons at once and scrolls pagewise through all icons.
         * The BarWidth should be the length of your screen
         * */
        VPetLCDMenuBar32p::VPetLCDMenuBar32p( uint8_t _numberOfMenuItems, uint8_t _iconsPerPage, uint16_t _barWidth){
            numberOfMenuItems=_numberOfMenuItems;
            barWidth=_barWidth;
            barHeight=SPRITES_MENU_RESOLUTION;//standard is the height of the icons
            iconsPerPage=_iconsPerPage;


            iconColor=0x64EA;
            selectionColor=0x0001;
            activeIconColor=0x0001;

            iconIndices = new uint8_t[numberOfMenuItems];
            activeFlags = new bool[numberOfMenuItems];
            currentSelection = -1; // Original logic expects no selected icon before the first NEXT press.

            for(int i =0; i<numberOfMenuItems;i++){
                iconIndices[i]=0;
                activeFlags[i]=false;
            }
        }

        /**
         *  Changes to the next Icon.
         * 
         * */
        void VPetLCDMenuBar32p::nextSelection(){
            currentSelection++;
            if(currentSelection == numberOfMenuItems){
                currentSelection =-1;
            }
        }


        /**
         * Sets the selections to the given index.
         * */
        void VPetLCDMenuBar32p::setSelection(uint8_t menuItemIndex){
            if(menuItemIndex >=0 && menuItemIndex < numberOfMenuItems){
                currentSelection = menuItemIndex;
            }
        }

        /**
         * Sets the icon for the menu entry. MenuItemIndex is the index of the menu entry, menuItemIconIndex is the  index of the symbol of the menu item in the HighResMenuitems.h 
         * */
        void VPetLCDMenuBar32p::setIconOnIndex(uint8_t menuItemIndex, uint8_t menuItemIconIndex){
                iconIndices[menuItemIndex] = menuItemIconIndex;
        }




        /**
         * Makes an icon visually active/darker without selecting it.
         * Used for the S3 notification icon: faint = no need, dark = needs attention.
         */
        void VPetLCDMenuBar32p::setIconActive(uint8_t menuItemIndex, bool active){
            if(menuItemIndex < numberOfMenuItems){
                activeFlags[menuItemIndex] = active;
            }
        }

        void VPetLCDMenuBar32p::clearActiveIcons(){
            for(int i=0; i<numberOfMenuItems; i++){
                activeFlags[i] = false;
            }
        }

        /**
         * Draws the Menu to the given Lcd
         * */
        void VPetLCDMenuBar32p::drawMenu(VPetLCD* lcd){
            const int16_t iconWidth = SPRITES_MENU_RESOLUTION;

            // Stage 1F2: TTGO T-Display S3 is 320 px wide in landscape.
            // 8 icons across 320 px = 40 px per slot. A 32 px icon gets 4 px side margin.
            // Keep formula generic: (320 - 32) / 7 = 41 px if edge-aligned,
            // but using 40 px slots gives a cleaner visual rhythm across the whole bar.
            int16_t iconStep = iconWidth;
            if (iconsPerPage > 1) {
                if (barWidth >= 320 && iconsPerPage == 8) {
                    iconStep = 40;
                } else {
                    iconStep = (int16_t)(barWidth - iconWidth) / (int16_t)(iconsPerPage - 1);
                    if (iconStep < 1) iconStep = iconWidth;
                }
            }
            const int16_t iconY = (barWidth >= 320) ? 4 : 0;

            uint8_t page = currentSelection / iconsPerPage;
            uint8_t indexOnPage = currentSelection % iconsPerPage;

            for(int i=0; i < iconsPerPage; i++){
                uint8_t currentIndex = page * iconsPerPage + i;
                if(currentIndex < numberOfMenuItems){
                    int16_t x = i * iconStep;
                    if (x + iconWidth > barWidth) x = barWidth - iconWidth;
                    uint16_t drawColor = activeFlags[currentIndex] ? activeIconColor : iconColor;
                    lcd->drawMenuItem(iconIndices[currentIndex], x + ((barWidth >= 320 && iconsPerPage == 8) ? 4 : 0), iconY, 1, false, drawColor);
                }
            }

            if (currentSelection > -1) {
                const int16_t offset = 2;
                int16_t x = indexOnPage * iconStep + offset + ((barWidth >= 320 && iconsPerPage == 8) ? 4 : 0);
                if (x + iconWidth > barWidth) x = barWidth - iconWidth;
                lcd->drawMenuItem(iconIndices[currentSelection], x, iconY + offset, 1, false, selectionColor);
            }

        }