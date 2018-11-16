//
// Created by marc on 15.06.18.
//

#ifndef DEEPKIT_TEXTCURSOR_H
#define DEEPKIT_TEXTCURSOR_H

#include <SkStrikeCache.h>
#include <SkGlyphCache.h>
#include <SkUtils.h>

#include <tuple>
#include <string>

SkUnichar SkUTF8_PrevUnichar(const char** ptr) {
    SkASSERT(ptr && *ptr);

    const char* p = *ptr;

    if (*--p & 0x80) {
        while (*--p & 0x40) {
            ;
        }
    }

    *ptr = (char*)p;
    return SkUTF8_NextUnichar(&p);
}

namespace Pesto {

    class TextCursor {
    public:
        std::string text;
        char *start;
        const char *current;
        const char *end;
        const SkPaint *paint;

        bool preserveWhiteSpace = false;
        bool breakOnNewLine = false;
        bool autoWrap = false;

        float whiteSpaceSize = 0.0f;

        SkExclusiveStrikePtr exklusiveCache;
        SkGlyphCache *glyphCache;

        explicit TextCursor(SkPaint &paint, std::string &text): TextCursor(paint, text.c_str()) {
        }

        explicit TextCursor(SkPaint &paint, const char *text) {
            exklusiveCache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(paint);
            glyphCache = exklusiveCache.get();
            this->paint = &paint;

            this->text = text;
            start = &this->text[0];
            current = start;
            end = &this->text[this->text.size()];

            whiteSpaceSize = paint.measureText(" ", 1);
        }

        void setPreserveWhiteSpace(bool preserveWhiteSpace) {
            this->preserveWhiteSpace = preserveWhiteSpace;
        }

        void setBreakOnNewLine(bool breakOnNewLine) {
            this->breakOnNewLine = breakOnNewLine;
        }

        void setAutoWrap(bool autoWrap) {
            this->autoWrap = autoWrap;
        }

        float widthOf(const char *character) {
            const char *cur = character;

            return glyphCache->getUnicharMetrics(SkUTF8_NextUnichar(&cur)).fAdvanceX;
        }

//        bool isWordSeparator(const char *chr) {
//            return ' ' == *chr || '\n' == *chr;
//        }

        bool isWhiteSpace(const char *chr) {
            return ' ' == *chr || '\t' == *chr;
        }

        bool isNewLine(const char *chr) {
            return '\n' == *chr || '\r' == *chr;
        }

        /**
         * Returns true if
         *   - preserveWhiteSpace is deactivated and chr is a white-space
         *   - breakOnNewLine is deactivated and chr is a new-line
         *
         * We don't need white-space or new-line in the output of next*Wrap(), so
         * this indicates whether they can be removed or not.
         */
        bool isUnnecessaryCharacter(const char *chr) {
            if (!preserveWhiteSpace && isWhiteSpace(chr)) {
                return true;
            }

            if (!breakOnNewLine && isNewLine(chr)) {
                return true;
            }

            return false;
        }

        /**
         * Returns always the next word (or part of a word, since 'sharp' wrap) until we reach a space.
         */
        std::tuple<float, std::string> nextSharpWrap(float maxWidth) {
            float width = 0.0f;

            //eat until we have a non-whitespace
            while (!preserveWhiteSpace && current < end && (isWhiteSpace(current) || isNewLine(current))) {
                current++;
            }

            const char *startedAt = current;

            while (current < end) {
                if (!preserveWhiteSpace && isWhiteSpace(current)) {
                    //if we remove white-space, we return immediately when we hit one.
                    //next round will eat up until a word.
                    break;
                }

                float glyphWidth = glyphCache->getUnicharMetrics(SkUTF8_NextUnichar(&current)).fAdvanceX;
                width += glyphWidth;

                if (width > maxWidth) {
                    //we're done, maxWidth over exhausted
                    //go back to last glyph

                    if (width == glyphWidth) {
                        //when are still at the start and already hit maxWidth, so just return that one character
                        //we can't use `startedAt == current` here, since `current` has already been increased by one char
                    } else {
                        width -= glyphWidth;
                        SkUTF8_PrevUnichar(&current);
                    }

                    break;
                }
            }

            return std::make_tuple(width, std::string(startedAt).substr(0, (current - startedAt)));
        }
        void reset() {
            current = start;
        }

        void reset(std::string text) {
            current = start;

            this->text = text;
            start = &this->text[0];
            current = start;
            end = &this->text[this->text.size()];
        }

        /**
         * Returns always one word or empty whitespace block.
         * If nothing is catched, return at least one character.
         */
        std::tuple<float, std::string> nextWordWrap(float maxWidth) {
            float width = 0.0f;

            //eat until we have a non-whitespace
            while (!preserveWhiteSpace && current < end && (isWhiteSpace(current) || isNewLine(current))) {
                current++;
            }

            const char *startedAt = current;
            bool hitNonWhitespace = false;

            while (current < end  && (isWhiteSpace(current) || isNewLine(current))) {
                //we add always at least the whitespace given
                width += whiteSpaceSize;
                current++;
            }

            if (width < maxWidth || startedAt == current) {
                //after collecting whitespace block, we have still room for a character.

                //we are in a word now (since we ate all whitespace)
                //so break at the first white-space or end
                while (current < end) {
                    if (isWhiteSpace(current) || isNewLine(current)) {
                        //we hit end of word
                        break;
                    }

                    float glyphWidth = glyphCache->getUnicharMetrics(SkUTF8_NextUnichar(&current)).fAdvanceX;
                    width += glyphWidth;
                }
            }

            return std::make_tuple(width, std::string(startedAt).substr(0, (current - startedAt)));
        }

        bool isEnd() {
            return current == end;
        }

        std::tuple<float, std::string> nextLine(float maxWidth, bool forceOneWord = false) {
            float lineWidth = 0.0f;
            std::string lineText;

            if (maxWidth < 0.f) {
                maxWidth = 0.0f;
            }

            float widthLeft = maxWidth;

            while (true) {
                float width = 0.0f;
                std::string text;

                //make sure we handle the new lines
                const char *reader = current;
                while (breakOnNewLine && reader < end) {
                    if (isWhiteSpace(reader)) {
                        //just eat it temporarily
                        reader++;
                    } else if (isNewLine(reader)) {
                        //ok, we got one, exit with current state. The read white-space
                        //will be handled next round
                        if (preserveWhiteSpace) {
                            //make sure we add the white-spaces to the results
                            //since we preserve them.
                            unsigned long readWhiteSpaces = (current - reader);
                            lineWidth += readWhiteSpaces * whiteSpaceSize;
                            lineText += " ";
                        }

                        reader++;
                        current = reader;

                        return std::make_tuple(lineWidth, lineText);
                    } else {
                        //nothing to do here, since no white-space or newline
                        break;
                    }
                }

                std::tie(width, text) = nextWordWrap(widthLeft);
                if (!width) {
                    break;
                }

                float requiresThisWidth = width;

                if (!preserveWhiteSpace && lineWidth != 0.0f) {
                    //we add a space between last word and current, since we didn't read spaces when preserveWhiteSpace=false
                    requiresThisWidth += whiteSpaceSize;
                }

                bool requiresThisWord = forceOneWord && lineWidth == 0.0f;

                if (!requiresThisWord && this->autoWrap) {
                    //we check width only when we don't require this word.

                    if (lineWidth + requiresThisWidth > maxWidth) {
                        //we exceeded the maxWidth, so rewind current
                        current = current - (text.size());
                        break;
                    }
                }

                if (!preserveWhiteSpace && lineWidth != 0.0f) {
                    //Since we remove white-space, we need to add it back between words.
                    //but only for the not first word

                    lineText += " ";
                }

                widthLeft -= requiresThisWidth;
                lineWidth += requiresThisWidth;
                lineText += text;
            }

            return std::make_tuple(lineWidth, lineText);
        }

        float measureWidth(const char *text) {
            float width = 0.0f;
            const char *cur = text;

            while (*cur) {
                width += glyphCache->getUnicharMetrics(SkUTF8_NextUnichar(&cur)).fAdvanceX;
            }

            return width;
        }

        int characterCount() {
            const char *cur = start;
            int count = 0;

            while (cur < end) {
                SkUTF8_NextUnichar(&cur);
                count++;
            }

            return count;
        }

        float totalWidth() {
            float width = 0.0f;
            const char *cur = start;

            while (*cur) {
                width += glyphCache->getUnicharMetrics(SkUTF8_NextUnichar(&cur)).fAdvanceX;
            }

            return width;
        }

//        bool next() {
//            if (current == end) {
//                return false;
//            }
//
//            //SkUTF8_NextUnichar moves the pointer right depending on char size it finds
//            x += glyphCache->getUnicharMetrics(SkUTF8_NextUnichar(&current)).fAdvanceX;
//
//            return true;
//        }
//
//        bool prev() {
//            if (current == start) {
//                return false;
//            }
//
//            //SkUTF8_NextUnichar moves the pointer right depending on char size it finds
//            x -= glyphCache->getUnicharMetrics(SkUTF8_PrevUnichar(&current)).fAdvanceX;
//
//            return true;
//        }
    };

}


#endif //DEEPKIT_TEXTCURSOR_H
