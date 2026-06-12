#include "SampleRepository.h"
#include "../util/JsonHelper.h"
#include <algorithm>
#include <stdexcept>
#include <cctype>

SampleRepository::SampleRepository(const std::string& filePath)
    : filePath_(filePath) {
    load();
}

void SampleRepository::load() {
    auto j = JsonHelper::Load(filePath_);
    data_.clear();
    for (const auto& item : j)
        data_.push_back(Sample::fromJson(item));
}

void SampleRepository::save() {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& s : data_)
        j.push_back(s.toJson());
    JsonHelper::Save(filePath_, j);
}

void SampleRepository::add(const Sample& s) {
    for (const auto& existing : data_)
        if (existing.id == s.id)
            throw std::runtime_error("이미 존재하는 시료 ID입니다: " + s.id);
    data_.push_back(s);
    save();
}

std::vector<Sample> SampleRepository::findAll() const {
    return data_;
}

std::optional<Sample> SampleRepository::findById(const std::string& id) const {
    for (const auto& s : data_)
        if (s.id == id) return s;
    return std::nullopt;
}

std::vector<Sample> SampleRepository::findByName(const std::string& keyword) const {
    std::vector<Sample> result;
    // 대소문자 무시 검색 (ASCII 범위)
    std::string lkw = keyword;
    std::transform(lkw.begin(), lkw.end(), lkw.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    for (const auto& s : data_) {
        std::string lname = s.name;
        std::transform(lname.begin(), lname.end(), lname.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (lname.find(lkw) != std::string::npos)
            result.push_back(s);
    }
    return result;
}

void SampleRepository::update(const Sample& s) {
    for (auto& existing : data_) {
        if (existing.id == s.id) {
            existing = s;
            save();
            return;
        }
    }
    throw std::runtime_error("시료를 찾을 수 없습니다: " + s.id);
}
