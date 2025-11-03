#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>
using namespace std;
struct Item {
string name;
double value;
double weight;


bool divisible;
int priority;
Item(string n, double val, double wt, bool div, int p)
:name(n), value(val), weight(wt), divisible(div), priority(p){}
double ValuePerWeight() const{
return value/weight;
}
};
bool compare (const Item& a, const Item& b) {
if (a.priority == b.priority) {
return a.ValuePerWeight() > b.ValuePerWeight();
}
return a.priority < b.priority;
}
double fractional_Knapsack (vector<Item>& items, double capacity, double&
totalWeightCarried) {
sort (items.begin(), items.end(), compare);
cout << "\n Sorted Items (By priority and then my value/weight)\n";
cout << left << setw(20) << "Item"
<< setw(20) << "Weight"
<< setw(20) << "Value"
<< setw(20) << "Priority"
<< setw(20) << "Value/Weight"
<< setw(20) << "Type" << "\n";
for (const auto& item:items) {
cout << left << item.name
<< setw(20) << item.weight
<< setw(20) << item.value
<< setw(20) << item.priority
<< setw(20) << fixed << setprecision(2) << item.ValuePerWeight()
<< setw(20) << (item.divisible ? "Divisible" : "Indivisible") << "\n";
}
double totalValue = 0.0, totalWeightCarried = 0.0;

cout << "\n Items selected for transport: \n";
for (const auto& item : items) {
if (capacity <= 0) break;
if (item.divisible) {
double takenWeight = min (item.weight, capacity); //take as much as fits
double takenValue = item.ValuePerWeight() * takenWeight;
totalValue+=takenValue; //add to total utility
capacity -= takenWeight; //Reduce the remaining capacity
totalWeightCarried += takenWeight;
cout << "-" << item.name << ":" << takenWeight << "kg. Utilities:
" << takenValue << ", Priority: " << item.priority << "Type: Divisible\n";
}
else {
if (item.weight <= capacity) { //only add it if it fully fits
totalValue += item.value; //add it
capacity -= item.weight; //reduce the remaining capacity
totalWeightCarried += item.weight;
cout << "-" << item.name << ":" << item.weight << "kg, Utilities: " << item.value <<
", Priority: " << item.priority << ", Type: Indivisible\n";
}
}
}
return totalValue;
}
int main() {
vector<Item> items = {
Item("Medical Kits", 10, 100, false, 1),
Item("Food Packets", 20, 60, true, 3),
Item("Drinking Water", 30, 90, true, 2),
Item("Blankets", 15, 45, false, 3),
Item("Infant Formula", 5, 50, false, 1)
};
double capacity;
cout << "Enter maximum capacity of the boat (in kg)";

cin >> capacity;
double totalWeightCarried;
double maxValue = fractional_Knapsack(items, capacity, totalWeightCarried);
cout << "\n=======FINAL REPORT=======\n";
cout << "Total weight carried: " << fixed << setprecision(2) << totalWeightCarried << "kg\n";
cout << "Total utility value carried: " << fixed << setprecision(2) << maxValue << "units\n";
return 0;
}
