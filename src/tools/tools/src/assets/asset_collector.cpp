#include "halley/tools/assets/asset_collector.h"
#include "halley/tools/file/filesystem.h"
#include "halley/file/byte_serializer.h"
#include "halley/resources/metadata.h"
#include "halley/support/logger.h"
#include "halley/file/compression.h"

using namespace Halley;


AssetCollector::AssetCollector(const ImportingAsset& asset, const Path& dstDir, const std::vector<Path>& assetsSrc, ProgressReporter reporter)
	: asset(asset)
	, dstDir(dstDir)
	, assetsSrc(assetsSrc)
	, reporter(reporter)
{}

void AssetCollector::output(const String& name, AssetType type, const Bytes& data, Maybe<Metadata> metadata)
{
	output(name, type, gsl::as_bytes(gsl::span<const Byte>(data)), metadata);
}

void AssetCollector::output(const String& name, AssetType type, gsl::span<const gsl::byte> data, Maybe<Metadata> metadata)
{
	String id = name.replaceAll("_", "__").replaceAll("/", "_-_").replaceAll(":", "_c_");
	Path filePath = Path(toString(type)) / id;

	if (metadata && metadata->getString("asset_compression", "") == "deflate") {
		auto newData = Compression::deflate(data);
		Logger::logInfo("- Writing compressed asset: " + (dstDir / filePath) + " (" + String::prettySize(newData.size()) + ")");
		FileSystem::writeFile(dstDir / filePath, newData);
	} else {
		Logger::logInfo("- Writing asset: " + (dstDir / filePath) + " (" + String::prettySize(data.size_bytes()) + ")");
		FileSystem::writeFile(dstDir / filePath, data);	
	}

	AssetResource result;
	result.name = name;
	result.type = type;
	result.filepath = filePath.string();
	if (metadata) {
		result.metadata = metadata.get();
	}
	assets.emplace_back(result);
}

void AssetCollector::addAdditionalAsset(ImportingAsset&& additionalAsset)
{
	additionalAssets.emplace_back(std::move(additionalAsset));
}

bool AssetCollector::reportProgress(float progress, const String& label)
{
	return reporter(progress, label);
}

const Path& AssetCollector::getDestinationDirectory()
{
	return dstDir;
}

Bytes AssetCollector::readAdditionalFile(const Path& filePath)
{
	for (auto path : assetsSrc) {
		Path f = path / filePath;
		if (FileSystem::exists(f)) {
			additionalInputs.push_back(TimestampedPath(f, FileSystem::getLastWriteTime(f)));
			return FileSystem::readFile(f);
		}
	}
	throw Exception("Unable to find asset dependency: \"" + filePath.getString() + "\"");
}

const std::vector<AssetResource>& AssetCollector::getAssets() const
{
	return assets;
}

std::vector<ImportingAsset> AssetCollector::collectAdditionalAssets()
{
	return std::move(additionalAssets);
}

const std::vector<TimestampedPath>& AssetCollector::getAdditionalInputs() const
{
	return additionalInputs;
}
