#include <eventhandler.h>
#include <simulation-state.h>
#include <filesystem>
#include <util/logging.h>
#include <io/result-output.h>

void handleElectrodeSwitching(Event *currentEvent,
                              Electrodes &electr,
                              SolutionVector &v,
                              Simu &simu,
                              SimulationState &simulationState) {
    // SWITCHES ELECTRODE
    // GET SWITCHING EVENT DATA
    SwitchingInstance *si = static_cast<SwitchingInstance *> ( currentEvent->getEventDataPtr());
    assert(si != nullptr);

    // If time stepping, reduce step size to minimum
    if (simu.simulationMode() == TimeStepping) {
        simulationState.dt(simu.getMindt());
    }

    // IF SWITCHING INSTANCE IS A FLAG FOR UNIFORM ELECTRIC FIELD, CAN EXIT
    if (si->electrodeNumber == SwitchingInstance::UNIFORM_E_FIELD) {
        return;
    }

    // SET THE NEW ELECTRODE VALUE. FIRST CHECK THAT
    electr.setElectrodePotential(si->electrodeNumber, si->potential);

    // SET POTENTIAL BOUNDARY CONDITIONS FOR ALL ELECTRODES
    v.setFixedNodesPot(&electr);
    v.setToFixedValues();
}

/**
 * THIS IS ONLY CALLED BEFORE SIMULATION STARTS, DOES NOT
 * NEED TO BE AS GENERAL AS handleEvents.
 * TAKES CARE OF:
 *     PRE-REFINEMENT
 *     CALCULATING INITIAL POTENTIALS
 *     OUTPUT result_initial FILE
 */
void handleInitialEvents(SimulationState &simulationState, // non-const since dt may change_
                         EventList &evel,
                         Electrodes &electrodes,
                         Alignment &alignment,
                         Simu &simu,
                         Geometries &geometries,
                         SolutionVectors &solutionvectors,
                         const LC &lc,
                         Settings &settings,
                         SpaMtrix::IRCMatrix &Kpot,
                         SpaMtrix::IRCMatrix &Kq,
                         ResultOutput &resultOutput) {

    int currentIteration = simulationState.currentIteration();
    double currentTime = simulationState.currentTime();
    double timeStep = simulationState.dt();
// IF NEEDS PRE-REFINEMENT. DO IT FIRST
    bool refineMesh = false;

    std::list<Event *> refEvents;    // REFINEMENT EVENTS EXECUTED TOGETHER
    while (evel.eventOccursNow(simulationState)) {
        Event *currentEvent = evel.getCurrentEvent(simulationState); // removes event from queue to be processed
        EventType et = currentEvent->getEventType();
        // REMOVE EVENT FROM LIST AND GET ITS TYPE
        ///EventType et = evel.popCurrentEvent( simu );

        // DEPENDING ON EVENT TYPE, DO STUFF
        switch (et) {
            case (EVENT_SAVE): // INITIAL RESULT IS ALWAYS WRITTEN. SEE BELOW
                delete currentEvent;
                break;
            case (EVENT_SWITCHING):  // SWITCH ELECTRODES
                handleElectrodeSwitching(currentEvent,
                                         electrodes,
                                         *solutionvectors.v,
                                         simu,
                                         simulationState);
                delete currentEvent; // NOT NEEDED ANYMORE
                break;
            case (EVENT_REFINEMENT): // REFINE MESH
                refEvents.push_back(currentEvent);
                refineMesh = true;
                break;
            default:
                throw std::runtime_error(fmt::format("Unknown event type in {}, {}.", __FILE__, __func__));
        }
    }
    if (refineMesh) {
        handlePreRefinement(refEvents,
                            geometries,
                            solutionvectors,
                            simu,
                            simulationState,
                            alignment,
                            electrodes,
                            lc.S0(),
                            Kpot,
                            Kq); // defined in refinementhandler.cpp
    }

    // ALWAYS CALCULATE INITIAL POTENTIAL
    calcpot3d(Kpot,
              solutionvectors.v,
              solutionvectors.q,
              lc,
              *geometries.geom,
              &settings,
              &electrodes);

    // WRITE INITIAL RESULT FILE. ALWAYS!
    /*
    handleResultOutput(simulationState,
                       simu,
                       lc.S0(),
                       *geometries.geom,
                       *solutionvectors.v,
                       *solutionvectors.q);
                       */
    Log::info("Writing initial results");
    resultOutput.writeResults(*geometries.geom, *solutionvectors.v, *solutionvectors.q, simulationState);

    // ADD REOCCURRING EVENTS
    evel.manageReoccurringEvents(currentIteration, currentTime, timeStep);
}

/*!
 * Reduces time step if necessary so that next time step does not skip over next event.
 */
void reduceTimeStep(SimulationState &simulationState, EventList &eventList) {
    double dt = simulationState.dt();
    double currentTime = simulationState.currentTime();
    assert(dt > 0);

    // FIND TIME UNTIL NEXT EVENT
    double tNext = eventList.timeUntilNextEvent(currentTime);
    assert(tNext > 0); // next time step should be in future

    if (tNext < dt) {
        simulationState.dt(tNext);
    }
}

void handleEvents(EventList &evel,
                  Electrodes &electrodes,
                  Alignment &alignment,
                  Simu &simu,
                  SimulationState& simulationState, // non-const, dt may be modified
                  Geometries &geometries,
                  SolutionVectors &solutionvectors,
                  const LC &lc,
                  Settings &settings,
                  SpaMtrix::IRCMatrix &Kpot,
                  SpaMtrix::IRCMatrix &Kq,
                  ResultOutput &resultOutput
) {
    //int currentIteration = simulationState.currentIteration();

// LEAVE IF NO EVENTS LEFT IN QUEUE
    if (!evel.eventsInQueue()) {    // event queue is empty
        evel.manageReoccurringEvents(simulationState.currentIteration(),
                                     simulationState.currentTime(),
                                     simulationState.dt());
        if (simu.simulationMode() == TimeStepping) {
            reduceTimeStep(simulationState, evel);
        }
        return;
    }

    // EVENTS ARE ORDERED BY TIME/ITERATION NUMBER,
    // BUT NOT ACCORDING TO TYPE. HOWEVER, DIFFERENT
    // EVENT TYPES SHOULD ALSO BE EXECUTED IN PARTICULAR ORDER
    // (e.g. UPDATE POTENTIAL VALUES BEFORE SAVING NEW RESULT FILE)
    // USE FOLLOWING FLAGS TO DETERMINE THIS
    bool recalculatePotential = false;
    bool saveResult = false;
    bool refineMesh = false;

    // CHECK WHICH EVENTS ARE OCCURRING *NOW* AND SET CORRESPONDING
    // FLAGS + OTHER PRE-EVENT PROCESSING
    std::list<Event *> refEvents; // STORES REF-EVENTS THAT NEED TO BE EXECUTED

    while (evel.eventOccursNow(simulationState)) {
        // REMOVE EVENT FROM LIST AND GET ITS TYPE
        Event *currentEvent = evel.getCurrentEvent(simulationState); // removes event from queue to be processed
        EventType et = currentEvent->getEventType();

        // DEPENDING ON EVENT TYPE, DO STUFF
        switch (et) {
            case (EVENT_SAVE): // SAVE RESULTS
                saveResult = true;
                delete currentEvent; // NOT NEEDED ANYMORE
                break;
            case (EVENT_SWITCHING):  // SWITCH ELECTRODES
                handleElectrodeSwitching(currentEvent, electrodes, *solutionvectors.v, simu, simulationState);
                delete currentEvent; // NOT NEEDED ANYMORE
                recalculatePotential = true;

                if ((evel.getSaveIter() > 1) || (evel.getSaveTime() > 0)) // OUTPUT RESULT ON SWITCHING ITERATION
                    saveResult = true;

                break;
            case (EVENT_REFINEMENT): // REFINE MESH
                refineMesh = true;
                recalculatePotential = true;
                saveResult = true;
                refEvents.push_back(currentEvent);
                break;
            default:
                throw std::runtime_error(fmt::format("Unknown event type in {}, {}", __FILE__, __func__));
        }
    }

// ADDS REOCCURRING EVENTS TO QUEUE FOR NEXT ITERATION
    evel.manageReoccurringEvents(simulationState.currentIteration(),
                                 simulationState.currentTime(),
                                 simulationState.dt());

// IF MESH REFINEMENT
    if (refineMesh) {
        handleMeshRefinement(refEvents,
                             geometries,
                             solutionvectors,
                             simu,
                             simulationState,
                             alignment,
                             electrodes,
                             lc.S0(),
                             Kpot,
                             Kq); // defined in refinementhandler.cpp
    }

    if (recalculatePotential) {
        calcpot3d(Kpot,
                  solutionvectors.v,
                  solutionvectors.q,
                  lc,
                  *geometries.geom,
                  &settings,
                  &electrodes);
    }

    if (saveResult) {
      resultOutput.writeResults(*geometries.geom, *solutionvectors.v, *solutionvectors.q, simulationState);
    }

    if (simu.simulationMode() == TimeStepping) {
        reduceTimeStep(simulationState, evel);
    }
}//end void HandleEvents


