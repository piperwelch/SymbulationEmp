#include "../../lysis_mode/Phage.h"
#include "../../lysis_mode/Bacterium.h"

TEST_CASE("Phage Process", "[lysis]") {

    GIVEN("A world and some phage") {

        emp::Ptr<emp::Random> random;
        random.New(5);
        LysisWorld w(*random);
        LysisWorld * world = &w;
        SymConfigBase config;
        config.LYSIS(1);
        double int_val = -1;

        WHEN("Lysis is enabled and the Phage's burst timer >= the burst time") {
            emp::Ptr<Phage> p;
            p.New(random, world, &config, int_val);
            emp::Ptr<Phage> p1;
            p1.New(random, world, &config, int_val);
            emp::Ptr<Phage> p2;
            p2.New(random, world, &config, int_val);

            emp::Ptr<Bacterium> h;
            h.New(random, &w, &config);

            p->SetHost(h);
            int time = 15;
            p->SetBurstTimer(time);

            h->AddReproSym(p1);
            h->AddReproSym(p2);

            THEN("Lysis occurs, the host dies") {
                size_t location = 2;
                // double resources_per_host_per_update = 40;

                p->Process(location);

                bool host_dead = true;
                std::vector<emp::Ptr<Organism>> empty_syms = {};
                REQUIRE(h->GetDead() == host_dead);
                REQUIRE(h->GetReproSymbionts() == empty_syms);
            }

            p.Delete();
            h.Delete();
        }

        WHEN("Lysis is enabled and the Phage's burst timer < the burst time") {
            emp::Ptr<Bacterium> h;
            h.New(random, &w, &config);

            double phage_points = 10;
            emp::Ptr<Phage> p3;
            p3.New(random, world, &config, int_val);
            emp::Ptr<Phage> p4;
            p4.New(random, world, &config, int_val);
            emp::Ptr<Phage> p5;
            p5.New(random, world, &config, int_val);
            p3->SetPoints(phage_points);

            h->AddReproSym(p4);
            h->AddReproSym(p5);
            int num_host_orig_syms = 2;
            p3->SetHost(h);

            THEN("Phage reproduces, Host gains repro symbionts, Phage loses points") {
                size_t location = 2;
                // bool host_dead = false;

                p3->Process(location);

                REQUIRE(h->GetDead() == false);

                std::vector<emp::Ptr<Organism>> updated_repro_syms = h->GetReproSymbionts();

                REQUIRE( (int) updated_repro_syms.size() > num_host_orig_syms); // host gained repro syms

                REQUIRE(p3->GetPoints() < phage_points); // phage lost points


            }

            p3.Delete();
            h.Delete();

        }
        random.Delete();
    }
}

TEST_CASE("Phage Vertical Transmission", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(-1);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;
    config.LYSIS(1);

    WHEN("phage is lytic"){
        config.LYSIS_CHANCE(1);

        WHEN("Vertical Transmission is enabled"){
            world->SetVertTrans(1);
            double host_int_val = .5;
            double sym_int_val = -.5;

            emp::Ptr<Bacterium> h = new Bacterium(random, world, &config, host_int_val);
            emp::Ptr<Phage> p = new Phage(random, world, &config, sym_int_val);
            h->AddSymbiont(p);

            emp::Ptr<Bacterium> host_baby = emp::NewPtr<Bacterium>(random, world, &config, h->GetIntVal());
            long unsigned int expected_sym_size = host_baby->GetSymbionts().size();
            p->VerticalTransmission(host_baby);

            THEN ("Phage does not vertically transmit") {
                REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
            }
        }
         WHEN("Vertical Transmission is disabled"){
            world->SetVertTrans(0);
            double host_int_val = .5;
            double sym_int_val = -.5;

            emp::Ptr<Bacterium> h = new Bacterium(random, world, &config, host_int_val);
            emp::Ptr<Phage> p = new Phage(random, world, &config, sym_int_val);
            h->AddSymbiont(p);

            emp::Ptr<Bacterium> host_baby = emp::NewPtr<Bacterium>(random, world, &config, h->GetIntVal());
            long unsigned int expected_sym_size = host_baby->GetSymbionts().size();
            p->VerticalTransmission(host_baby);

            THEN ("Phage does not vertically transmit") {
                REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
            }
        }

    }

    WHEN("phage is lysogenic"){
        config.LYSIS_CHANCE(0);

        WHEN("Vertical Transmission is enabled"){
            world->SetVertTrans(1);
            double host_int_val = .5;
            double sym_int_val = -.5;

            emp::Ptr<Bacterium> h = new Bacterium(random, world, &config, host_int_val);
            emp::Ptr<Phage> p = new Phage(random, world, &config, sym_int_val);
            h->AddSymbiont(p);

            emp::Ptr<Bacterium> host_baby = emp::NewPtr<Bacterium>(random, world, &config, h->GetIntVal());
            long unsigned int expected_sym_size = host_baby->GetSymbionts().size() +1;
            p->VerticalTransmission(host_baby);

            THEN ("Phage does vertically transmit") {
                REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
            }
        }
         WHEN("Vertical Transmission is disabled"){
            world->SetVertTrans(0);
            double host_int_val = .5;
            double sym_int_val = -.5;

            emp::Ptr<Bacterium> h = new Bacterium(random, world, &config, host_int_val);
            emp::Ptr<Phage> p = new Phage(random, world, &config, sym_int_val);
            h->AddSymbiont(p);

            emp::Ptr<Bacterium> host_baby = emp::NewPtr<Bacterium>(random, world, &config, h->GetIntVal());
            long unsigned int expected_sym_size = host_baby->GetSymbionts().size()+1;
            p->VerticalTransmission(host_baby);

            THEN ("Phage does vertically transmit") {
                REQUIRE(host_baby->GetSymbionts().size() == expected_sym_size);
            }
        }

    }

}

TEST_CASE("Host phage death and removal from syms list", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(6);
    LysisWorld w(*random);
    LysisWorld *world = &w;
    SymConfigBase config;
    config.SYM_LIMIT(2);

    WHEN("there is a single lysogenic phage and it is dead"){
        config.LYSIS_CHANCE(0);
        double host_int_val = .5;
        double sym_int_val = -.5;

        emp::Ptr<Bacterium> h = new Bacterium(random, world, &config, host_int_val);
        emp::Ptr<Phage> p = new Phage(random, world, &config, sym_int_val);

        h->AddSymbiont(p);
        p->SetDead();

        long unsigned int expected_sym_size = 0;
        h->Process(0);

        THEN("phage is removed from syms list"){
            REQUIRE(h->GetSymbionts().size() == expected_sym_size);
        }
    }

    WHEN("There are multiple lysogenic phage and only one dies"){
        config.LYSIS_CHANCE(0);
        double host_int_val = .5;
        double sym_int_val = -.5;

        Bacterium * h = new Bacterium(random, world, &config, host_int_val);
        Phage * p1 = new Phage(random, world, &config, sym_int_val);
        Phage * p2 = new Phage(random, world, &config, 0.0);

        h->AddSymbiont(p1);
        h->AddSymbiont(p2);
        p1->SetDead();

        long unsigned int expected_sym_size = 1;
        h->Process(0);

        THEN("Only the dead phage is removed from the syms list"){
            REQUIRE(h->GetSymbionts().size() == expected_sym_size);

            Organism * curSym = h->GetSymbionts()[0];
            REQUIRE(curSym->GetIntVal() == p2->GetIntVal());
            REQUIRE(curSym == p2);
        }
    }
}

TEST_CASE("Phage LysisBurst", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(6);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;

    config.LYSIS(1);
    config.GRID_X(2);
    config.GRID_Y(1);
    config.SYM_LIMIT(10);
    int location = 0;

    double int_val = 0;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);

    GIVEN("create two hosts and add both to world as neighbors, add phage offspring to the original host's repro syms"){
        Bacterium * orig_h = new Bacterium(random, &w, &config, int_val);
        Bacterium * new_h = new Bacterium(random, &w, &config, int_val);
        orig_h->AddSymbiont(p);
        world->AddOrgAt(orig_h, 0);
        world->AddOrgAt(new_h, 1);

        emp::Ptr<Organism> p_baby1 = p->reproduce();
        emp::Ptr<Organism> p_baby2 = p->reproduce();
        orig_h->AddReproSym(p_baby1);
        orig_h->AddReproSym(p_baby2);

        WHEN("call the burst method so we can check injection") {
            int original_sym_num = new_h->GetSymbionts().size() + orig_h->GetSymbionts().size();
            p->LysisBurst(location);

            THEN("the repro syms go into the other host as symbionts and the original host dies"){
                int new_sym_num = new_h->GetSymbionts().size() + orig_h->GetSymbionts().size();
                REQUIRE(new_sym_num == original_sym_num+2);
                REQUIRE(size(orig_h->GetReproSymbionts()) == 0);
                REQUIRE(orig_h->GetDead() == true);
            }
        }
    }
}

TEST_CASE("Phage LysisStep", "[lysis]"){
    emp::Ptr<emp::Random> random = new emp::Random(9);
    LysisWorld w(*random);
    LysisWorld * world = &w;
    SymConfigBase config;

    double sym_repro_points = 5.0;
    config.LYSIS(1);
    config.SYM_LYSIS_RES(sym_repro_points);

    double int_val = 0;
    emp::Ptr<Phage> p = new Phage(random, world, &config, int_val);
    Bacterium * h = new Bacterium(random, &w, &config, int_val);
    h->AddSymbiont(p);

    WHEN("The phage doesn't have enough resources to reproduce"){
        double repro_syms_size_pre_process = size(h->GetReproSymbionts());
        double orig_points = 3.0;
        double expected_points = 3.0;
        p->SetPoints(orig_points);
        double orig_burst_time = 0.0;
        p->SetBurstTimer(orig_burst_time);

        p->LysisStep();
        THEN("The burst timer is incremented but no offspring are created"){
            REQUIRE(p->GetBurstTimer() > orig_burst_time);
            REQUIRE(size(h->GetReproSymbionts()) == repro_syms_size_pre_process);
            REQUIRE(p->GetPoints() == expected_points);
        }
    }

    WHEN("The phage does have enough resources to reproduce"){
        double expected_repro_syms_size_post_process = size(h->GetReproSymbionts()) + 1; //one offspring created
        double orig_points = sym_repro_points;//symbiont given enough resources to produce one offspring
        double expected_points = 0.0;
        p->SetPoints(orig_points);
        double orig_burst_time = 0.0;
        p->SetBurstTimer(orig_burst_time);

        p->LysisStep();
        THEN("The burst timer is incremented and offspring are created"){
            REQUIRE(p->GetBurstTimer() > orig_burst_time);
            REQUIRE(size(h->GetReproSymbionts()) == expected_repro_syms_size_post_process);
            REQUIRE(p->GetPoints() == expected_points);
        }
    }
}
