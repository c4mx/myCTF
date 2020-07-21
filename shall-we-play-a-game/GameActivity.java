package com.google.ctf.shallweplayagame;

import android.animation.AnimatorSet;
import android.os.Bundle;
import android.support.p021v7.app.C0433c;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.LinearLayout;
import android.widget.TextView;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.Random;

public class GameActivity extends C0433c implements OnClickListener {

    /* renamed from: l */
    C0676a[][] f2382l = ((C0676a[][]) Array.newInstance(C0676a.class, new int[]{3, 3}));

    /* renamed from: m */
    Queue<AnimatorSet> f2383m = new LinkedList();

    /* renamed from: n */
    // init random number
    Object f2384n = C0675N.secretFunc(Integer.valueOf(3), C0675N.f2396h, Long.valueOf((((((((1416127776 + 1869507705) + 544696686) + 1852403303) + 544042870) + 1696622963) + 544108404) + 544501536) + 1886151033));

    /* renamed from: o */
    int playTimes;

    /* renamed from: p */
    boolean dead;

    /* renamed from: q */
    byte[] randomArray = new byte[32];

    /* renamed from: r */
    byte[] f2388r = {-61, 15, 25, -115, -46, -11, 65, -3, 34, 93, -39, 98, 123, 17, 42, -121, 60, 40, -60, -112, 77, 111, 34, 14, -31, -4, -7, 66, 116, 108, 114, -122};

    public GameActivity() {

        // init first byte[] array
        C0675N.secretFunc(Integer.valueOf(3), C0675N.f2397i, this.f2384n, this.randomArray);
        this.playTimes = 0;
        this.dead = false;
    }

    /* access modifiers changed from: 0000 */
    /* renamed from: a */
    public C0676a mo3213a(List<C0676a> list) {
        return (C0676a) list.get(((Random) this.f2384n).nextInt(list.size()));
    }

    /* access modifiers changed from: 0000 */
    /* renamed from: a */

    // Check if game over
    public boolean checkContinue(C0680a aVar) {
        int[] iArr = {0, 0, 0};
        int[] iArr2 = {0, 0, 0};
        int[] iArr3 = {0, 0};
        for (int i = 0; i < 3; i++) {
            for (int i2 = 0; i2 < 3; i2++) {
                if (this.f2382l[i2][i].f2401d == aVar) {
                    iArr[i] = iArr[i] + 1;
                    iArr2[i2] = iArr2[i2] + 1;
                    if (i == i2) {
                        iArr3[0] = iArr3[0] + 1;
                    }
                    if (i + i2 == 2) {
                        iArr3[1] = iArr3[1] + 1;
                    }
                }
            }
        }
        for (int i3 : iArr) {
            if (i3 >= 3) {
                return true;
            }
        }
        for (int i4 : iArr2) {
            if (i4 >= 3) {
                return true;
            }
        }
        for (int i5 : iArr3) {
            if (i5 >= 3) {
                return true;
            }
        }
        return false;
    }

    /* access modifiers changed from: 0000 */
    /* renamed from: k */
    public void emptyGameBoard() {
        AnimatorSet animatorSet = (AnimatorSet) this.f2383m.poll();
        if (animatorSet != null) {
            animatorSet.start();
        }
    }

    /* access modifiers changed from: 0000 */
    /* renamed from: l */
    public List<C0676a> mo3216l() {
        ArrayList arrayList = new ArrayList();
        for (int i = 0; i < 3; i++) {
            for (int i2 = 0; i2 < 3; i2++) {
                if (this.f2382l[i2][i].mo3222a()) {
                    arrayList.add(this.f2382l[i2][i]);
                }
            }
        }
        return arrayList;
    }

    /* access modifiers changed from: 0000 */
    /* renamed from: m */

    // Called when the game is run 1,000,000 times
    public void showFlag() {
        // last calculate: 
        // - pass 5 arguments
        // - nested calling
        Object random = C0675N.secretFunc(Integer.valueOf(0), C0675N.f2389a, Integer.valueOf(0));
        C0675N.secretFunc(Integer.valueOf(0), C0675N.f2391c, random, Integer.valueOf(2), C0675N.secretFunc(Integer.valueOf(1), C0675N.f2390b, this.randomArray, Integer.valueOf(1)));
        
        // Show flag here
        ((TextView) findViewById(R.id.score)).setText(new String((byte[]) C0675N.secretFunc(Integer.valueOf(0), C0675N.f2392d, random, this.f2388r)));
        
        setDeathAnimation();
    }

    /* access modifiers changed from: 0000 */
    /* renamed from: n */

    // Run a new game
    public void newGame() {
        for (int i = 0; i < 3; i++) {
            for (int i2 = 0; i2 < 3; i2++) {
                this.f2382l[i2][i].mo3221a(C0680a.EMPTY, 25);
            }
        }
        emptyGameBoard();
        this.playTimes++;

        // new Game call
        // get a random
        Object random = C0675N.secretFunc(Integer.valueOf(2), C0675N.f2393e, Integer.valueOf(2));

        // update randomArray
        C0675N.secretFunc(Integer.valueOf(2), C0675N.f2394f, random, this.randomArray);
        
        // Change randomArray value
        this.randomArray = (byte[]) C0675N.secretFunc(Integer.valueOf(2), C0675N.f2395g, random);

        // Check play times
        if (this.playTimes == 1000000) {
            showFlag();
            return;
        }
        ((TextView) findViewById(R.id.score)).setText(String.format("%d / %d", new Object[]{Integer.valueOf(this.playTimes), Integer.valueOf(1000000)}));
    }

    /* access modifiers changed from: 0000 */
    /* renamed from: o */

    // Set death animation
    public void setDeathAnimation() {
        for (int i = 0; i < 3; i++) {
            for (int i2 = 0; i2 < 3; i2++) {
                this.f2382l[i2][i].setValue(C0680a.DEATH);
            }
        }
        this.playTimes = 0;
        this.dead = true;
        emptyGameBoard();
    }

    public void onClick(View view) {
        if (!this.dead && this.f2383m.isEmpty()) {
            C0676a aVar = (C0676a) view;
            if (!aVar.mo3222a()) {
                C0681b.m3227b();
                return;
            }
            C0681b.m3226a();
            // Me play:
            aVar.setValue(C0680a.X);
            if (checkContinue(C0680a.X)) {
                newGame();
                return;
            }

            // Check if no empty place
            List l = mo3216l();
            if (l.isEmpty()) {
                newGame();
                return;
            }
            // Computer play:
            mo3213a(l).setValue(C0680a.O);
            if (checkContinue(C0680a.O)) {
                setDeathAnimation();
            } else {
                emptyGameBoard();
            }
        }
    }

    /* access modifiers changed from: protected */
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        setContentView((int) R.layout.activity_game);
        LinearLayout linearLayout = (LinearLayout) findViewById(R.id.rows);
        for (int i = 0; i < 3; i++) {
            LinearLayout linearLayout2 = new LinearLayout(getApplicationContext());
            for (int i2 = 0; i2 < 3; i2++) {
                C0676a aVar = new C0676a(getApplicationContext(), this.f2383m);
                linearLayout2.addView(aVar);
                this.f2382l[i2][i] = aVar;
                aVar.setOnClickListener(this);
            }
            linearLayout.addView(linearLayout2);
        }
    }
}