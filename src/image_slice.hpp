#pragma once

#include "rect.hpp"

namespace browservice {

// Reference to rectangular part of shared image buffer
class ImageSlice {
public:
    // Create empty image slice
    ImageSlice()
        : buf_(nullptr),
          width_(0),
          height_(0),
          pitch_(0),
          globalX_(0),
          globalY_(0)
    {}

    // Create new independent width x height image buffer with background
    // color (r g b)
    static ImageSlice createImage(int width, int height, uint8_t r, uint8_t g, uint8_t b);
    static ImageSlice createImage(int width, int height, uint8_t rgb = 255);

    // Create new buffer from string rows and color mapping
    static ImageSlice createImageFromStrings(
        const vector<string>& rows,
        const map<char, array<uint8_t, 3>>& colors
    );

    // Returns pixel buffer pointer
    uint8_t* buf() { return buf_; }

    int width() { return width_; }
    int height() { return height_; };

    int pitch() { return pitch_; }

    // Coordinates of upper left corner of this slice in original shared
    // image buffer
    int globalX() { return globalX_; }
    int globalY() { return globalY_; }

    bool containsGlobalPoint(int gx, int gy) {
        int x = gx - globalX_;
        int y = gy - globalY_;
        return x >= 0 && y >= 0 && x < width_ && y < height_;
    }

    // Returns true if slice is empty ie contains 0 pixels
    bool isEmpty() {
        return width_ == 0 || height_ == 0;
    }

    // Get pointer to given pixel Does no bounds checking - can be used with
    // x = width to obtain past-the-end-of-line pointer
    uint8_t* getPixelPtr(int x, int y) {
        return &buf_[4 * (y * pitch_ + x)];
    }

    // Set pixel in slice to given RGB value If point (x y) is outside
    // slice does nothing
    void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        if(x >= 0 && x < width_ && y >= 0 && y < height_) {
            uint8_t* pos = getPixelPtr(x, y);
            *(pos + 2) = r;
            *(pos + 1) = g;
            *(pos + 0) = b;
        }
    }
    void setPixel(int x, int y, uint8_t rgb) {
        setPixel(x, y, rgb, rgb, rgb);
    }

    // Copy src to image slice at offset
    void putImage(ImageSlice src, int x, int y) {
        Rect rect = Rect::intersection(
            Rect(0, src.width_, 0, src.height_),
            Rect::translate(Rect(0, width_, 0, height_), -x, -y)
        );

        if(!rect.isEmpty()) {
            for(int lineY = rect.startY; lineY < rect.endY; ++lineY) {
                const uint8_t* srcLine = src.getPixelPtr(rect.startX, lineY);
                uint8_t* destLine = getPixelPtr(rect.startX + x, lineY + y);
                memcpy(destLine, srcLine, 4 * (rect.endX - rect.startX));
            }
        }
    }

    // Get clamped subrectangle slice
    ImageSlice subRect(int startX, int endX, int startY, int endY) {
        clampBoundX_(startX);
        clampBoundX_(endX);
        clampBoundY_(startY);
        clampBoundY_(endY);
        endX = max(endX, startX);
        endY = max(endY, startY);

        ImageSlice ret = *this;
        ret.width_ = endX - startX;
        ret.height_ = endY - startY;
        ret.buf_ += 4 * (startY * ret.pitch_ + startX);
        ret.globalX_ += startX;
        ret.globalY_ += startY;
        return ret;
    }

    // Split slice by clamped coordinate
    pair<ImageSlice, ImageSlice> splitX(int x) {
        return {
            subRect(0, x, 0, height_),
            subRect(x, width_, 0, height_)
        };
    }
    pair<ImageSlice, ImageSlice> splitY(int y) {
        return {
            subRect(0, width_, 0, y),
            subRect(0, width_, y, height_)
        };
    }

    // Fill clamped subrectangle with color
    void fill(
        int startX, int endX, int startY, int endY,
        uint8_t r, uint8_t g, uint8_t b
    ) {
        clampBoundX_(startX);
        clampBoundX_(endX);
        clampBoundY_(startY);
        clampBoundY_(endY);
        endX = max(endX, startX);
        endY = max(endY, startY);

        for(int y = startY; y < endY; ++y) {
            uint8_t* pos = getPixelPtr(startX, y);
            for(int x = startX; x < endX; ++x) {
                *(pos + 2) = r;
                *(pos + 1) = g;
                *(pos + 0) = b;
                pos += 4;
            }
        }
    }
    void fill(int startX, int endX, int startY, int endY, uint8_t rgb) {
        clampBoundX_(startX);
        clampBoundX_(endX);
        clampBoundY_(startY);
        clampBoundY_(endY);
        endX = max(endX, startX);
        endY = max(endY, startY);

        for(int y = startY; y < endY; ++y) {
            uint8_t* startPos = getPixelPtr(startX, y);
            uint8_t* endPos = getPixelPtr(endX, y);
            std::fill(startPos, endPos, rgb);
        }
    }

    // Clone slice to new independent buffer
    ImageSlice clone() {
        ImageSlice ret;

        ret.globalBuf_.reset(new vector<uint8_t>());
        ret.globalBuf_->reserve(4 * width_ * height_);
        for(int y = 0; y < height_; ++y) {
            ret.globalBuf_->insert(
                ret.globalBuf_->end(), getPixelPtr(0, y), getPixelPtr(width_, y)
            );
        }

        ret.buf_ = ret.globalBuf_->data();
        ret.width_ = width_;
        ret.height_ = height_;
        ret.pitch_ = width_;
        ret.globalX_ = 0;
        ret.globalY_ = 0;
        return ret;
    }

private:
    void clampBoundX_(int& x) {
        x = max(min(x, width_), 0);
    }
    void clampBoundY_(int& y) {
        y = max(min(y, height_), 0);
    }

    shared_ptr<vector<uint8_t>> globalBuf_;

    uint8_t* buf_;

    int width_;
    int height_;

    int pitch_;

    int globalX_;
    int globalY_;
};

}
