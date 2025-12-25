cmake_minimum_required(VERSION 3.16)
project(HEP_Generation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# HEP libraries installed under /opt/hep in the container
set(PREFIX "/opt/hep")

include_directories(${PREFIX}/include)
link_directories(${PREFIX}/lib ${PREFIX}/lib64)

# --- Basic Pythia Test ---
add_executable(gen_pythia src/gen_pythia.cc)
target_link_libraries(gen_pythia PRIVATE pythia8 HepMC3 HepMC3search)

# --- Angantyr Heavy Ion Test ---
add_executable(gen_angantyr src/gen_angantyr.cc)
target_link_libraries(gen_angantyr PRIVATE pythia8 HepMC3 HepMC3search)

# --- B+ -> K+ J/psi Signal Generator ---
add_executable(gen_bpkjpsi src/gen_bpkjpsi.cc)
target_link_libraries(gen_bpkjpsi PRIVATE 
    pythia8 EvtGen EvtGenExternal
    Photospp PhotosppHepMC3
    TauolaCxxInterface TauolaFortran
    HepMC3 HepMC3search
    gfortran
)

# --- D0 Spin Alignment Study (Pb-Pb) ---
add_executable(gen_d0_study src/gen_d0_study.cc)
target_link_libraries(gen_d0_study PRIVATE 
    pythia8 EvtGen EvtGenExternal
    Photospp PhotosppHepMC3
    TauolaCxxInterface TauolaFortran
    HepMC3 HepMC3search
    gfortran
)

# --- HepMC3 Spin Analyzer ---
add_executable(analyze_spin src/analyze_spin.cc)
target_link_libraries(analyze_spin PRIVATE HepMC3)

# --- Prompt J/psi Generator (OniaShower) ---
add_executable(gen_prompt_jpsi src/gen_prompt_jpsi.cc)
target_link_libraries(gen_prompt_jpsi PRIVATE pythia8 HepMC3 HepMC3search)
