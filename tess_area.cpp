#define CSV_IO_NO_THREAD
#include "csv.h"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

struct Grain {
  double x, y;
  double m11, m12, m21, m22;
  double s11, s12, s21, s22;
  double ta; // tess_area;
};

int main(int argc, char **argv) {
  std::string inputfile, outputfile;
  if (argc == 2) {
    inputfile = argv[1];
  } else {
    std::cerr << "Incorret number of arguments." << std::endl;
    std::exit(EXIT_FAILURE);
  }
  outputfile = inputfile;
  outputfile.erase(outputfile.find_last_of("."), std::string::npos);
  outputfile.append("_stress.txt");
  std::ifstream infile(inputfile);
  std::ofstream ofs(outputfile);
  // Including the x, y positions, moment M11/12/21/22 of grain

  std::vector<double> radius;
  std::vector<std::pair<double, double>> Vert;
  std::vector<Grain> grains;
  std::unordered_map<unsigned int, std::pair<double, double>> vertices;
  std::unordered_map<unsigned int, std::vector<std::pair<double, double>>>
      connects;
  io::CSVReader<2> inC("Connectivity.data");
  io::CSVReader<3> inV("Vertices.data");
  io::CSVReader<1> inR("Radius.data");
  double vertice_x, vertice_y, radii, grain_x, grain_y, dx, dy, density,
      void_area, wd = 0., ht = 0.;
  double M11, M12, M21, M22, ignored;
  double max_x =
      0.; // The postion of the furthest grain(used to define the flowfront)
  double tess_area = 0.,
         g_area = 0.; // area is of single cell;Area is the Tesselaton area
  signed int connect;
  unsigned int i = 1, j = 1, n = 0, cell, degree;

  while (inV.read_row(ignored, vertice_x, vertice_y))
    if (ignored == j) {
      vertices.insert(
          std::make_pair(ignored, std::make_pair(vertice_x, vertice_y)));
      ++j;
    }
  // only store the positive vertice id numbering

  while (inR.read_row(radii))
    radius.emplace_back(radii);

  // Maximum radius in order to define the flowfront(~15d=30max_radius)
  double max_radius = *(std::max_element(radius.begin(), radius.end()));

  // Read the x, y and moment component from inputfile(DEM*.dat)
  std::string line;
  if (infile.is_open()) {
    while (std::getline(infile, line)) {
      if (line != "") {
        std::istringstream istream(line);
        istream >> ignored >> grain_x >> grain_y >> M11 >> M12 >> M21 >> M22;
        Grain g;
        g.x = grain_x;
        g.y = grain_y;
        if (grain_x > max_x)
          max_x = grain_x;
        g.m11 = M11;
        g.m12 = M12;
        g.m21 = M21;
        g.m22 = M22;
        grains.emplace_back(g);
      }
    }
  }
  infile.close();

  while (inC.read_row(cell, connect)) {
    if (i == cell && connect > 0) {
      // only concern vertices inside the domain whose id numbering is greater
      // than 0;
      auto xandy = vertices.find(connect)->second;
      Vert.emplace_back(xandy);
    } else if (i < cell) {
      connects.insert(std::make_pair(i, Vert));
      Vert.clear();
      ++i;
      // Insert the vector of vertices into the map of connects with the index i
      // and flush the vector to store vertices of the next cell;
      if (connect > 0) {
        auto xandy = vertices.find(connect)->second;
        Vert.emplace_back(xandy);
      }
    }
  }
  connects.insert(std::make_pair(i, Vert)); // The vertices of the last cell
                                            // should be inserted after the
                                            // while loop;
  int k = connects.size();

  // Calculate the area of each cell stored in connects map
  for (int n = 0; n < k; ++n) {
    int in = n + 1;

    double area = 0, length, dg_area;
    std::vector<std::pair<double, double>> temp = connects.find(in)->second;
    // Enumerate the vertices vector by "it"
    dg_area = M_PI * std::pow(radius[n], 2);
    for (auto m = temp.begin(); m != temp.end(); ++m) {
      auto next = m + 1;
      if (m == (temp.end() - 1)) {
        next = temp.begin();
      }
      dx = (next->first) - (m->first);
      dy = (next->second) - (m->second);
      length = std::pow((dx * dx + dy * dy), 0.5);
      if (length > 4 * radius[n]) {
        area = 0;
        dg_area = 0;
        break;
      }
      area += ((m->first) * (next->second)) - ((next->first) * (m->second));
    }
    g_area += dg_area;
    tess_area += std::fabs(area) / 2;
    grains[n].ta = tess_area;
    if (tess_area != 0) {
      grains[n].s11 = grains[n].m11 / tess_area;
      grains[n].s12 = grains[n].m12 / tess_area;
      grains[n].s21 = grains[n].m21 / tess_area;
      grains[n].s22 = grains[n].m22 / tess_area;
    } else {
      grains[n].s11 = 0;
      grains[n].s12 = 0;
      grains[n].s21 = 0;
      grains[n].s22 = 0;
    }
  }

  void_area += tess_area - g_area;
  density = g_area / tess_area;

  double flowfront_x = max_x - 30 * max_radius;
  if (ofs.is_open()) {
    ofs << std::setprecision(5) << "x, " << std::setprecision(5)
        << "Nomalised x to 15d," << std::setprecision(5) << "Tesslation Area,"
        << std::setprecision(5) << "S11," << std::setprecision(5) << "S12,"
        << std::setprecision(5) << "S21," << std::setprecision(5) << "S22"
        << "\n";
    for (auto grain : grains) {
      if (grain.x >= flowfront_x&&grain.y <=max_radius)
        // For entire sliding mass the above line can be deleted!
        ofs << std::setprecision(5) << grain.x << ", " << std::setprecision(5)
            << (grain.x - flowfront_x) / (30 * max_radius) << ", "
            << std::setprecision(5) << grain.ta << ", " << std::setprecision(5)
            << grain.s11 << ", " << std::setprecision(5) << grain.s12 << ", "
            << std::setprecision(5) << grain.s21 << ", " << std::setprecision(5)
            << grain.s22 << "\n";
    }
    ofs.close();
  }
}
