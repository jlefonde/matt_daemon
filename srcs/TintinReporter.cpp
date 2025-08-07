#include "TintinReporter.hpp"

TintinReporter::TintinReporter(const std::string& article_name, const std::string& publication_file)
{
    this->article_name_ = article_name;
    this->publication_file_ = publication_file;
}

TintinReporter::TintinReporter(const TintinReporter& reporter)
{
    this->article_name_ = reporter.article_name_;
    this->publication_file_ = reporter.publication_file_;
}

TintinReporter::~TintinReporter() {}

TintinReporter& TintinReporter::operator=(const TintinReporter& reporter)
{
    if (this != &reporter)
    {
        this->article_name_ = reporter.article_name_;
        this->publication_file_ = reporter.publication_file_;
    }

    return *this;
}

void TintinReporter::log(LogLevel log_level, const char *article)
{

}
