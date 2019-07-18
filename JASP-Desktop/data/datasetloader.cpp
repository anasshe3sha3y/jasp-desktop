//
// Copyright (C) 2013-2018 University of Amsterdam
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "datasetloader.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "sharedmemory.h"
#include "dataset.h"

#include "importers/csvimporter.h"
#include "importers/jaspimporter.h"
#include "importers/odsimporter.h"
#include "importers/readstatimporter.h"

#include <QFileInfo>

using namespace std;
using namespace ods;
using namespace boost::interprocess;
using namespace boost;

string DataSetLoader::getExtension(const string &locator, const string &extension) {
	filesystem::path path(locator);
	string ext = path.extension().generic_string();

	if (!ext.length()) ext=extension;
	return ext;
}

Importer* DataSetLoader::getImporter(DataSetPackage *packageData, const string &locator, const string &extension)
{
	Importer* result = NULL;
	string ext = getExtension(locator, extension);

	if (boost::iequals(ext,".csv") || boost::iequals(ext,".txt"))	result = new CSVImporter(packageData);
	else if(boost::iequals(ext,".ods"))								result = new ODSImporter(packageData);
	else if(ReadStatImporter::extSupported(ext))					result = new ReadStatImporter(packageData, ext);


	return result; //If NULL then JASP will load it as a .jasp file
}

void DataSetLoader::loadPackage(DataSetPackage *packageData, const string &locator, const string &extension, boost::function<void(const string &, int)> progress)
{
	Importer* importer = getImporter(packageData, locator, extension);

	if (importer)
	{
		importer->loadDataSet(locator, progress);
		delete importer;
	}
	else if(extension == ".jasp" || extension == "jasp")
		JASPImporter::loadDataSet(packageData, locator, progress);
	else
		throw std::runtime_error("JASP does not support the file-type " + extension);
}

void DataSetLoader::syncPackage(DataSetPackage *packageData, const string &locator, const string &extension, boost::function<void(const string &, int)> progress)
{
	Importer* importer = getImporter(packageData, locator, extension);

	if (importer)
	{
		importer->syncDataSet(locator, progress);
		delete importer;
	}
}

void DataSetLoader::freeDataSet(DataSet *dataSet)
{
	SharedMemory::deleteDataSet(dataSet);
}

