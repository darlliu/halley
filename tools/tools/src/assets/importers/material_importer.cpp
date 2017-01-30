#include "material_importer.h"
#include "halley/file/byte_serializer.h"
#include "halley/core/graphics/material/material_definition.h"
#include "halley/support/exception.h"
#include <yaml-cpp/yaml.h>
#include "halley/tools/file/filesystem.h"
#include "halley/text/string_converter.h"

using namespace Halley;

void MaterialImporter::import(const ImportingAsset& asset, IAssetCollector& collector)
{
	Path basePath = asset.inputFiles.at(0).name.parentPath();
	auto material = parseMaterial(basePath, gsl::as_bytes(gsl::span<const Byte>(asset.inputFiles.at(0).data)), collector);
	collector.output(material.getName(), AssetType::MaterialDefinition, Serializer::toBytes(material));
}

MaterialDefinition MaterialImporter::parseMaterial(Path basePath, gsl::span<const gsl::byte> data, IAssetCollector& collector) const
{
	MaterialDefinition material;
	String strData(reinterpret_cast<const char*>(data.data()), data.size());
	YAML::Node root = YAML::Load(strData.cppStr());

	// Load base material
	if (root["base"]) {
		String baseName = root["base"].as<std::string>();
		auto otherData = collector.readAdditionalFile(basePath / baseName);
		material = parseMaterial(basePath, gsl::as_bytes(gsl::span<Byte>(otherData)), collector);
	}

	// Load name
	material.name = root["name"] ? root["name"].as<std::string>() : "Unknown";

	// Load attributes & uniforms
	if (root["attributes"]) {
		loadAttributes(material, root["attributes"]);
	}
	if (root["uniforms"]) {
		loadUniforms(material, root["uniforms"]);
	}

	// Load passes
	for (auto passNode : root["passes"]) {
		loadPass(material, passNode.as<YAML::Node>());
	}

	return material;
}

void MaterialImporter::loadPass(MaterialDefinition& material, const YAML::Node& node)
{
	String blend = node["blend"].as<std::string>("Opaque");
	BlendType blendType;
	if (blend == "Opaque") {
		blendType = BlendType::Opaque;
	}
	else if (blend == "Alpha") {
		blendType = BlendType::Alpha;
	}
	else if (blend == "AlphaPremultiplied") {
		blendType = BlendType::AlphaPremultiplied;
	}
	else {
		throw Exception("Unknown blend type: " + blend);
	}

	material.passes.emplace_back(MaterialPass(blendType, node["vertex"].as<std::string>(""), node["geometry"].as<std::string>(""), node["pixel"].as<std::string>("")));
}

void MaterialImporter::loadUniforms(MaterialDefinition& material, const YAML::Node& topNode)
{
	auto attribSeqNode = topNode.as<YAML::Node>();
	for (auto attribEntry : attribSeqNode) {
		for (YAML::const_iterator it = attribEntry.begin(); it != attribEntry.end(); ++it) {
			String name = it->first.as<std::string>();
			ShaderParameterType type = parseParameterType(it->second.as<std::string>());

			material.uniforms.push_back(MaterialAttribute(name, type, -1));
		}
	}
}

ShaderParameterType MaterialImporter::parseParameterType(String rawType)
{
	if (rawType == "float") {
		return ShaderParameterType::Float;
	}
	else if (rawType == "vec2") {
		return ShaderParameterType::Float2;
	}
	else if (rawType == "vec3") {
		return ShaderParameterType::Float3;
	}
	else if (rawType == "vec4") {
		return ShaderParameterType::Float4;
	}
	else if (rawType == "mat4") {
		return ShaderParameterType::Matrix4;
	}
	else if (rawType == "sampler2D") {
		return ShaderParameterType::Texture2D;
	}
	else {
		throw Exception("Unknown attribute type: " + rawType);
	}
}

void MaterialImporter::loadAttributes(MaterialDefinition& material, const YAML::Node& topNode)
{
	int location = int(material.attributes.size());
	int offset = material.vertexStride;

	auto attribSeqNode = topNode.as<YAML::Node>();
	for (auto attribEntry : attribSeqNode) {
		for (YAML::const_iterator it = attribEntry.begin(); it != attribEntry.end(); ++it) {
			String name = it->first.as<std::string>();
			ShaderParameterType type = parseParameterType(it->second.as<std::string>());

			material.attributes.push_back(MaterialAttribute());
			auto& a = material.attributes.back();
			a.name = name;
			a.type = type;
			a.location = location++;
			a.offset = offset;

			int size = getAttributeSize(type);
			offset += size;

			if (a.name == "a_vertPos") {
				material.vertexPosOffset = a.offset;
			}
		}
	}

	material.vertexStride = offset;
}

int MaterialImporter::getAttributeSize(ShaderParameterType type)
{
	switch (type) {
	case ShaderParameterType::Float: return 4;
	case ShaderParameterType::Float2: return 8;
	case ShaderParameterType::Float3: return 12;
	case ShaderParameterType::Float4: return 16;
	default: throw Exception("Unknown type: " + toString(int(type)));
	}
}
