//
//  main.cpp
//  IREP
//
//  Created by Ilan Moscovitz on 11/28/17.
//  Copyright © 2017 Ilan Moscovitz. All rights reserved.
//

#include <iostream>
#include <string>
#include <cmath>  
#include <vector>
#include <fstream>
#include <array>
#include "Rule.h"
#include "RuleSet.h"
#include "Dataset.h"
#include "Example.h"

using namespace std;

double gain(Rule rule0, Rule rule1, Dataset data) {
    int p0count = rule0.numCovered(data.posexamples);
    int p1count = rule1.numCovered(data.posexamples);
    int n0count = rule0.numCovered(data.negexamples);
    int n1count = rule1.numCovered(data.negexamples);

    return (double) p1count * (log2((double) (p1count + 1) / (p1count + n1count + 1)) - log2((double) (p0count + 1) / (p0count + n0count + 1)));
}

Rule growRule(Dataset trainset) {
    Rule rule0;
    Rule successorsteps(trainset);
    double bestactiongain;
    int bestactionindex;
    while (rule0.numCovered(trainset.negexamples) > 0) {
        Rule rule1 = rule0;
        bestactionindex = - 1;
        bestactiongain = 0;
        for (int r = 0; r < successorsteps.conds.size(); r++) {
            rule1.conds.push_back(successorsteps.conds[r]);
            double g = gain(rule0, rule1, trainset);
            if (g > bestactiongain) {
                bestactionindex = r;
                bestactiongain = g;
            }
            rule1.conds.pop_back();
        }
        rule0.conds.push_back(successorsteps.conds[bestactionindex]);
        successorsteps.conds.erase(successorsteps.conds.begin() + bestactionindex);
    }
    return rule0;
}

RuleSet IREP(Dataset trainset) {
    RuleSet ruleset;
    while (trainset.posexamples.size() > 0) {
        Rule newrule = growRule(trainset);
        Performance newruleperform = newrule.evaluateOn(trainset);
        if (newruleperform.accuracy < .50) {
            return ruleset;
        }
        int i = 0;
        while (i < trainset.posexamples.size()) {
            if (newrule.coversExample(trainset.posexamples[i])) {
                trainset.posexamples.erase(trainset.posexamples.begin() + i);
            } else {
                i++;
            }
        }
        ruleset.rules.push_back(newrule);
    }
    return ruleset;
}

int main() {

    srand( (unsigned) time(0));
    Dataset data;
    data.load("votes.dta");
    
    int n = 5;
    float accuracyavg = 0;
    for (int i = 0; i < n; i++) {
        cout << "\nRUN #" << i + 1 << "\n";
        array<Dataset, 2> splitdata = data.split(.75);
        RuleSet ruleset = IREP(splitdata[0]);
        ruleset.fancyoutput();
        Performance performance = ruleset.evaluateOn(splitdata[1]);
        performance.output();
        accuracyavg += (float) performance.accuracy / n;
    }
    cout << "{\"Average Accuracy\":" << accuracyavg <<"}";

    return 0;
}
